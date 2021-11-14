#include "div.h"
#include "functions.h"
#include "x.h"
#include <X11/X.h>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <stdio.h>
#include "image.h"

State state = {
    .num_active_images = 1,
    .ignore_mask = Mod2Mask | LockMask,
    .xevent_mask = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_STRUCTURE_NOTIFY,
    .right_to_left = 0,
    .scale_mode = SCALE_HEIGHT,
    .zoom = 1,
};

void onStartup();

void default_window_title() {
    static char buffer[255];
    if(getNumActiveImages() == 1)
        snprintf(buffer, sizeof(buffer) -1, "%s %d/%d", image_holders->name, state.file_index + 1, state.num_files);
    else
        snprintf(buffer, sizeof(buffer) -1, "%s %d-%d of %d", image_holders->name, state.file_index + 1, state.file_index + 1 + getNumActiveImages() - 1, state.num_files);
    setWindowTitle(buffer);
}

void default_open_images() {
    for (int i = 0; i < getNumActiveImages(); i++) {
        image_holders[i].image_data = openImage(state.image_context, state.file_index + i, image_holders[i].image_data);
        if(!image_holders[i].image_data) {
            image_holders[i].name = NULL;
            continue;
        }
        image_holders[i].image_width = getImageWidth(image_holders[i].image_data);
        image_holders[i].image_height = getImageHeight(image_holders[i].image_data);
        image_holders[i].name = getImageName(image_holders[i].image_data);
        image_holders[i].raw = getRawImage(image_holders[i].image_data);


    }
    state.num_files = getImageNum(state.image_context);
}

void default_parse_options(int argc, const char **argv) {
    for(argv++; argv[0]; argv++){
        if(argv[0][0] != '-')
            break;
        if(argv[0][1] == '-' && !argv[0][2]) {
            argv++;
            break;
        }
        argv = defaultSingleArgParse(argv);
    }
    if(argv[0]) {
        state.file_names = argv;
    }
}

void create_image_context() {
    state.image_context = createContext(state.file_names, state.num_files, REMOVE_INVALID );
}

void (*events[LAST_EVENT])() = {
    [XCB_KEY_PRESS] = onKeyPress,
    [XCB_CONFIGURE_NOTIFY] = onConfigureEvent,

    [ON_STARTUP] = onStartup,

    [PROCESS_ARGS] = default_parse_options,
    [POST_XCONNECTION] = create_image_context,
    [RENDER] = render,
    [OPEN_IMAGES] = default_open_images,
    [SET_TITLE] = default_window_title,
};

Binding bindings[] = {
    {0, XK_q, exit, 0},
    {0, XK_f, cycleScaleMode, 1},
    {ShiftMask, XK_f, cycleScaleMode, -1},
    {0, XK_semicolon, cycleAlignment, 1},
    {ShiftMask, XK_semicolon, cycleAlignment, -1},
    {0, XK_minus, zoom,        -1},
    {ShiftMask, XK_plus, zoom, +1},
    {0, XK_equal, zoom,        0},
    {0, XK_d, toggle_multi_page,        2},
    {0, XK_t, toggle_multi_page,        16},
    {0, XK_m, toggle_right_to_left,     },
    {0, XK_g, toggle_grid,    4 },

    {0, XK_j, next_image,  +1},
    {0, XK_k, next_image,  -1},
    {0, XK_space, next_page,  +1},
    {ShiftMask, XK_space, next_page,  -1},
    {0}
};
