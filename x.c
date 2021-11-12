#include <assert.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <Imlib2.h>
#include <X11/Xlib-xcb.h>

#include <X11/X.h>
#include <X11/keysym.h>
#include "div.h"
#include "x.h"

xcb_connection_t* dis;
Display* dpy;
xcb_gcontext_t gc;

xcb_atom_t pid_atom;
xcb_atom_t wm_name_atom;
xcb_atom_t utf8_string_atom;

xcb_generic_event_t* event;

Binding* all_bindings[] = {user_bindings, bindings};

xcb_atom_t get_atom(xcb_connection_t* dis, const char*name) {
    xcb_intern_atom_reply_t* reply;
    reply = xcb_intern_atom_reply(dis, xcb_intern_atom(dis, 0, strlen(name), name), NULL);
    xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

xcb_window_t createWindow(xcb_connection_t* dis, xcb_window_t parent) {
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (dis));
    xcb_screen_t* screen = iter.data;

    xcb_window_t win = xcb_generate_id(dis);

    xcb_window_t pid = xcb_generate_id (dis);
    state.drawable =pid;
    xcb_create_pixmap(dis, screen->root_depth, pid, screen->root, 5000, 5000);

    gc = xcb_generate_id (dis);

    uint32_t gc_values [] = {state.fg_color, state.bg_color};
    xcb_create_gc (dis, gc, screen->root, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND , gc_values);

    uint32_t values [] = {pid, state.xevent_mask};
    xcb_create_window(dis, XCB_COPY_FROM_PARENT, win, parent? parent: screen->root, 0, 0, GET(state.win_width, 200), GET(state.win_height, 200), 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
 screen->root_visual, XCB_CW_BACK_PIXMAP | XCB_CW_EVENT_MASK, &values);
    return win;
}

void setWindowHints(xcb_connection_t* dis, xcb_window_t win) {
    xcb_icccm_wm_hints_t hints;
    xcb_icccm_wm_hints_set_input(&hints, 1);
    xcb_icccm_wm_hints_set_normal(&hints);
    xcb_icccm_set_wm_hints_checked(dis, win, &hints);
}

void setWindowClass(xcb_connection_t* dis, xcb_window_t win) {
    const char* instance_name = GET(state.name, initial_args[0]);
    const char* class_name =  CLASSNAME;
    char class_instance[strlen(class_name) + strlen(instance_name) + 2];
    strcpy(class_instance, instance_name);
    strcpy(class_instance + strlen(instance_name) + 1, class_name);
    xcb_icccm_set_wm_class(dis, win, LEN(class_instance), class_instance);
}

void setWindowProperties(xcb_connection_t* dis, xcb_window_t win) {
    setWindowHints(dis, win);
    pid_t pid = getpid();
    xcb_change_property(dis, XCB_PROP_MODE_REPLACE, win, pid_atom, XCB_ATOM_CARDINAL, 32, 1, &pid);
    setWindowClass(dis, win);
}

void openXConnection() {
    dpy = XOpenDisplay(NULL);
    if(!dpy) {
        exit(2);
    }
    dis = XGetXCBConnection(dpy);
    //dis = xcb_connect(NULL, NULL);
    if(xcb_connection_has_error(dis))
        exit(2);
    XSetEventQueueOwner(dpy, XCBOwnsEventQueue);

    pid_atom = get_atom(dis, "_NET_WM_PID");
    wm_name_atom = get_atom(dis, "_NET_WM_NAME");
    utf8_string_atom = get_atom(dis, "UTF8_STRING");
}

uint32_t setupXConnection() {
    openXConnection();
    xcb_window_t wid = createWindow(dis, 0);// support embedded
    setWindowProperties(dis, wid);
    addNewEventFD(xcb_get_file_descriptor(dis), POLLIN, processXEvents);
    xcb_map_window(dis, wid); // TODO move later

    imlib_context_set_display(dpy);
    imlib_context_set_visual(DefaultVisual(dpy, DefaultScreen(dpy)));
    imlib_context_set_colormap(DefaultColormap(dpy, DefaultScreen(dpy)));
    imlib_context_set_drawable(state.drawable);
    return wid;
}

void clear_drawable(xcb_window_t wid, uint16_t width, uint16_t height) {
    xcb_rectangle_t rect = {0,0,width, height};
    xcb_poly_fill_rectangle(dis, wid, gc, 1, &rect);
}
void clear_window(xcb_window_t wid, uint16_t width, uint16_t height) {
    xcb_clear_area(dis, 0, wid, 0, 0, width,  height);
}
void flush() {
    xcb_flush(dis);
}

void initlizeBindings() {
    xcb_key_symbols_t * symbols = xcb_key_symbols_alloc(dis);
    for(int n = 0; n < LEN(all_bindings); n++)
        for (int i = 0; all_bindings[n] && all_bindings[n][i].func; i++) {
            if(all_bindings[n][i].keysym < 8 )
                all_bindings[n][i].keycode = all_bindings[n][i].keysym;
            else {
                xcb_keycode_t * codes = xcb_key_symbols_get_keycode(symbols, all_bindings[n][i].keysym);
                all_bindings[n][i].keycode = codes[0];
                free(codes);
            }
        }
    xcb_key_symbols_free(symbols);
}

void processXEvent(xcb_generic_event_t* event) {
    void (*func)()=events[event->response_type & 127];
    if(func)
        func();
}

void processXEvents() {
    while(event = xcb_poll_for_event(dis)) {
        processXEvent(event);
        free(event);
    }
}

bool processQueuedXEvents() {
    bool processedEvent = 0;
    while(event = xcb_poll_for_queued_event(dis)) {
        processXEvent(event);
        free(event);
        processedEvent = 1;
    }
    return processedEvent;
}

void onKeyPress() {
    xcb_keycode_t detail = ((xcb_key_press_event_t*)event)->detail;
    uint8_t mod = ((xcb_key_press_event_t*)event)->state & ~state.ignore_mask;
    for(int n = 0; n < LEN(all_bindings); n++)
        for (int i = 0; all_bindings[n][i].func; i++) {
            if(all_bindings[n][i].keycode == detail && all_bindings[n][i].mod == mod) {
                all_bindings[n][i].func(all_bindings[n][i].arg);
                return;
            }
        }
}

void onConfigureEvent() {
    xcb_configure_notify_event_t* configure_event = (xcb_configure_notify_event_t*)event;
    if (configure_event->window == state.wid) {
        state.win_width = configure_event->width;
        state.win_height = configure_event->height;
    }
}

void setWindowTitle(const char* str) {
    xcb_change_property(dis, XCB_PROP_MODE_REPLACE, state.wid, wm_name_atom, utf8_string_atom , 8, strlen(str), str);
    xcb_change_property(dis, XCB_PROP_MODE_REPLACE, state.wid, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(str), str);
}
