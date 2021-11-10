#include <sys/poll.h>

#include "div.h"
#include "functions.h"
#include "image.h"
#include "x.h"
const char** initial_args;
int initial_num_args;

void parse_options(int argc, const char **argv) {
    state.file_names = argv + 1;
    state.num_files= argc - 1;
}

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
        do
            RUN_EVENT(POST_EVENT);
        while(processQueuedXEvents());
        state.event_counter++;
    }
}

ImageHolder image_holders[MAX_IMAGES];

void render() {
    clear_window(state.wid, state.win_width, state.win_height);
    img_render(image_holders, state.num_active_images, state.wid, state.win_width, state.win_height);
}

int main(int argc, const char **argv) {
    initial_args = argv;
    initial_num_args = argc;
    RUN_EVENT(ON_STARTUP);
    RUN_EVENT(PROCESS_ARGS, argc, argv);
    state.wid = setupXConnection();
    RUN_EVENT(POST_XCONNECTION);
    open_images();
    doEventLoop();
    return 0;
}
