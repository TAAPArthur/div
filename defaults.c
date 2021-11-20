#include <X11/X.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <xcb/xcb.h>

#include "div.h"
#include "functions.h"
#include "image.h"
#include "image_view.h"
#include "x.h"

State state = {
    .ignore_mask = Mod2Mask | LockMask,
    .xevent_mask = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_STRUCTURE_NOTIFY,
    .default_binding_type = XCB_KEY_PRESS,
    .right_to_left = 0,
    .scale_mode = SCALE_HEIGHT,
    .zoom = 1,
    .user_title = CLASSNAME,
};

void (*scaleFunc)(const char* buf, uint32_t original_width, uint32_t original_height, char* out_buf, uint32_t width, uint32_t height, int num_channels) = nearestNeighbourScale;

void onStartup();

void defaultWindowTitle() {
    static char buffer[255];
    if(getNumActiveImages() == 1)
        snprintf(buffer, sizeof(buffer) - 1, "%s %s %d/%d", state.user_title, image_holders->name, state.file_index + 1, state.num_files);
    else
        snprintf(buffer, sizeof(buffer) - 1, "%s %s %d-%d of %d", state.user_title, image_holders->name, state.file_index + 1, state.file_index + 1 + getNumActiveImages() - 1, state.num_files);
    setWindowTitle(buffer);
}

void defaultOpenImages() {
    for (int i = 0; i < getNumActiveImages() && i < getImageNum(state.image_context); i++) {
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

void defaultParseOptions(int argc, const char **argv) {
    bool stop = 0;
    for(argv++; argv[0]; argv++){
        if(argv[0][0] != '-')
            break;
        if(argv[0][1] == '-' && !argv[0][2]) {
            argv++;
            break;
        }
        argv = defaultSingleArgParse(argv, &stop);
        if(stop)
            break;
    }
    if(argv[0]) {
        state.file_names = argv;
    }
}

void createImageContext() {
    state.image_context = createContext(state.file_names, state.num_files, REMOVE_INVALID );
}

void (*events[LAST_EVENT])() = {
    [XCB_KEY_PRESS] = onKeyPress,
    [XCB_CONFIGURE_NOTIFY] = onConfigureEvent,

    [ON_STARTUP] = onStartup,

    [PROCESS_ARGS] = defaultParseOptions,
    [PRE_MAP_WINDOW] = setWindowProperties,
    [POST_XCONNECTION] = createImageContext,
    [RENDER] = render,
    [OPEN_IMAGES] = defaultOpenImages,
    [SET_TITLE] = defaultWindowTitle,
};

Binding bindings[] = {
    {ShiftMask, XK_q, exit, 4},
    {0, XK_q, exit, 0},
    {0, XK_f, cycleScaleMode, 1},
    {ShiftMask, XK_F, cycleScaleMode, -1},
    {0, XK_semicolon, cycleAlignment, 1},
    {ShiftMask, XK_semicolon, cycleAlignment, -1},
    {0, XK_minus, zoom,        -1},
    {ShiftMask, XK_plus, zoom, +1},
    {0, XK_equal, zoom,        0},
    {0, XK_d, toggle_column,        2},
    {0, XK_t, toggle_grid,        2},
    {ShiftMask, XK_T, toggle_grid,        4},
    {0, XK_m, toggle_right_to_left,     },
    {0, XK_g, jump_start,     },
    {ShiftMask, XK_G, jump_end,     },

    {0, XK_j, next_image,  +1},
    {0, XK_k, next_image,  -1},
    {0, XK_space, next_page,  +1},
    {ShiftMask, XK_space, next_page,  -1},
    {0}
};
