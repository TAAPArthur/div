#include <X11/X.h>
#include <assert.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_keysyms.h>

#include "image_view.h"
#include "div.h"
#include "x.h"

xcb_connection_t* dis;
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

int depth;
xcb_window_t createWindow(xcb_connection_t* dis, xcb_window_t parent) {
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (dis));
    xcb_screen_t* screen = iter.data;

    xcb_window_t win = xcb_generate_id(dis);

    xcb_window_t pid = xcb_generate_id (dis);
    state.drawable =pid;
    xcb_create_pixmap(dis, screen->root_depth, pid, screen->root, 5000, 5000);

    depth = screen->root_depth;

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

void openXConnection() {
    dis = xcb_connect(NULL, NULL);
    if(xcb_connection_has_error(dis))
        exit(2);

    pid_atom = get_atom(dis, "_NET_WM_PID");
    wm_name_atom = get_atom(dis, "_NET_WM_NAME");
    utf8_string_atom = get_atom(dis, "UTF8_STRING");
}

void closeXConnection() {
    xcb_disconnect(dis);
    dis = NULL;
}

uint32_t setupXConnection() {
    openXConnection();
    xcb_window_t wid = createWindow(dis, state.parent);
    state.wid = wid;
    RUN_EVENT(PRE_MAP_WINDOW, wid);
    addNewEventFD(xcb_get_file_descriptor(dis), POLLIN, processXEvents);
    xcb_map_window(dis, wid); // TODO move later
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


void setWindowProperties(xcb_window_t win) {
    setWindowHints(dis, win);
    pid_t pid = getpid();
    xcb_change_property(dis, XCB_PROP_MODE_REPLACE, win, pid_atom, XCB_ATOM_CARDINAL, 32, 1, &pid);
    setWindowClass(dis, win);
}

void initlizeBindings() {
    xcb_key_symbols_t * symbols = xcb_key_symbols_alloc(dis);
    for(int n = 0; n < LEN(all_bindings); n++)
        for (int i = 0; all_bindings[n] && all_bindings[n][i].func; i++) {
            if(!all_bindings[n][i].type)
                all_bindings[n][i].type = state.default_binding_type;
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
    while(dis && (event = xcb_poll_for_event(dis))) {
        processXEvent(event);
        free(event);
    }
}

bool processQueuedXEvents() {
    bool processedEvent = 0;
    while(dis && (event = xcb_poll_for_queued_event(dis))) {
        processXEvent(event);
        free(event);
        processedEvent = 1;
    }
    return processedEvent;
}

void onKeyPress() {
    xcb_keycode_t detail = ((xcb_key_press_event_t*)event)->detail;
    uint8_t mod = ((xcb_key_press_event_t*)event)->state & ~state.ignore_mask;
    uint8_t type = ((xcb_key_press_event_t*)event)->response_type;
    for(int n = 0; n < LEN(all_bindings); n++)
        for (int i = 0; all_bindings[n][i].func; i++) {
            if( (!all_bindings[n][i].keycode || all_bindings[n][i].keycode == detail) &&
                    (all_bindings[n][i].mod == AnyModifier || all_bindings[n][i].mod == mod) &&
                    (!all_bindings[n][i].type || all_bindings[n][i].type == -1 || all_bindings[n][i].type == type)
                    ) {
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

int isXConnectionOpen() {
    if(dis && xcb_connection_has_error(dis))
        closeXConnection();

    return !!dis;
}

void img_render(ImageInfo*holder, int num, uint32_t wid, uint32_t win_width, uint32_t win_height) {
	uint32_t dw, dh;
    uint32_t total_image_width = 0, total_image_height = 0;

    dw = (win_width - state.padding_x *2) / getCols();
    dh = (win_height- state.padding_y *2) / getRows();

    for (int i = 0; i < getCols() && i < num; i++) {
        total_image_width += get_effective_dim(holder[i].image_width, holder[i].image_height, dw, dh, state.scale_mode, 0) + holder[i].padding_x;
    }

    for (int i = 0; i < num; i+=getCols()) {
        total_image_height += get_effective_dim(holder[i].image_width, holder[i].image_height, dw, dh, state.scale_mode, 1) + holder[i].padding_y;
    }

    int startingX = state.padding_x + adjustAlignment(state.align_mode_x, total_image_width, win_width);
    int y = state.padding_y + adjustAlignment(state.align_mode_y, total_image_height, win_height);

    uint32_t effective_width;
    uint32_t effective_height;
    for (int r = 0, i = 0; r < getRows(); r++) {
        int x = state.right_to_left ? win_width - startingX : startingX;
        for (int c = 0; c < getCols() && i < num; c++, i++) {
            if(!holder[i].image_data || !holder[i].raw)
                continue;

            float zoom = state.zoom ? state.zoom : 1;
            void * default_image_data = NULL;
            if(!scaleFunc || (zoom == 1 && state.scale_mode == SCALE_NORMAL)) {
                effective_width = holder[i].image_width;
                effective_height = holder[i].image_height;
                zoom = 1;
                default_image_data = holder[i].raw;
            } else {
                effective_width = get_effective_dim(holder[i].image_width, holder[i].image_height, dw, dh, state.scale_mode, 0) ;
                effective_height = get_effective_dim(holder[i].image_width, holder[i].image_height, dw, dh, state.scale_mode, 1);
            }

            xcb_image_t *image = xcb_image_create_native(dis,effective_width,effective_height,XCB_IMAGE_FORMAT_Z_PIXMAP ,depth,NULL, 0, default_image_data );
            if(!image)
                return;
            if(scaleFunc)
                scaleFunc(holder[i].raw, holder[i].image_width, holder[i].image_height, image->data, effective_width*zoom , effective_height*zoom, 4);

            if(zoom > 1 || effective_width > win_width || effective_height > win_height) {
                xcb_image_t *sub_image = xcb_image_subimage(image,holder[i].offset_x, holder[i].offset_y, MIN(effective_width, win_width) ,MIN(effective_height, win_height),NULL,0,NULL);
               xcb_image_destroy(image);
               image = sub_image;
            }

            xcb_image_put(dis, wid, gc, image , x - (state.right_to_left? effective_width: 0), y, 0);
            xcb_image_destroy(image);

            x+=(effective_width + holder[i].padding_x) * (state.right_to_left?-1:1);
        }
        y += effective_height;
    }
}
