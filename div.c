#include <poll.h>
#include <string.h>

#include "div.h"
#include "functions.h"
#include "x.h"
#include "image_view.h"
const char** initial_args;
int initial_num_args;
ImageInfo image_holders[MAX_IMAGES];

struct {
    struct pollfd pollFDs[255];
    void(*extraEventCallBacks[255])();
    int numberOfFDsToPoll;
} eventFDInfo;

int addNewEventFD(int fd, short events, void (*callback)()) {
    eventFDInfo.pollFDs[eventFDInfo.numberOfFDsToPoll] = (struct pollfd) {fd, events};
    eventFDInfo.extraEventCallBacks[eventFDInfo.numberOfFDsToPoll] = callback;
    return eventFDInfo.numberOfFDsToPoll++;
}

void removeExtraEvent(int index) {
    for(int i = index + 1; i < eventFDInfo.numberOfFDsToPoll; i++) {
        eventFDInfo.pollFDs[i - 1] = eventFDInfo.pollFDs[i];
        eventFDInfo.extraEventCallBacks[i - 1] = eventFDInfo.extraEventCallBacks[i];
    }
    eventFDInfo.numberOfFDsToPoll--;
}

int processEvents(int timeout) {
    int numEvents;
    if((numEvents = poll(eventFDInfo.pollFDs, eventFDInfo.numberOfFDsToPoll, timeout))) {
        for(int i = eventFDInfo.numberOfFDsToPoll - 1; i >= 0; i--) {
            if(eventFDInfo.pollFDs[i].revents) {
                if(eventFDInfo.pollFDs[i].revents & eventFDInfo.pollFDs[i].events) {
                    eventFDInfo.extraEventCallBacks[i](eventFDInfo.pollFDs[i].fd, eventFDInfo.pollFDs[i].revents);
                }
                if(eventFDInfo.pollFDs[i].revents & (POLLERR | POLLNVAL | POLLHUP)) {
                    removeExtraEvent(i);
                }
            }
        }
    }
    return numEvents;
}

void doEventLoop() {
    while(eventFDInfo.numberOfFDsToPoll) {
        processEvents(-1);
        if(!isXConnectionOpen())
            continue;
        RUN_EVENT(POST_EVENT);
        do
            maybe_render();
        while(processQueuedXEvents());
        flush();
    }
}

void maybe_render() {
    static State last_state;
    static uint32_t last_num_active_images;
    bool diff_num_images = getNumActiveImages() != last_num_active_images;
    if(diff_num_images) {
        open_images();
        last_num_active_images= getNumActiveImages();
    }
    if(diff_num_images || memcmp(&last_state, &state, sizeof(State))) {
        RUN_EVENT(RENDER);
        RUN_EVENT(SET_TITLE);
        last_state = state;
    }
}

void render() {
    clear_drawable(state.drawable, state.win_width, state.win_height);
    img_render(image_holders, getNumActiveImages(), state.drawable, state.win_width, state.win_height);
    clear_window(state.wid, state.win_width, state.win_height);
    state.dirty = 0;
}

int main(int argc, const char **argv) {
    initial_args = argv;
    initial_num_args = argc;
    RUN_EVENT(ON_STARTUP);
    RUN_EVENT_WITH_ARGS(PROCESS_ARGS, argc, argv);
    setupXConnection();
    initlizeBindings();
    RUN_EVENT(POST_XCONNECTION);
    open_images();
    doEventLoop();
    return 0;
}
