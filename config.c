#include "config.h"
#include "div.h"
#include "functions.h"
#include "x.h"
#include <X11/X.h>
#include <X11/keysym.h>
#include <xcb/xcb.h>

State state = {
    .num_active_images = 2,
    .xevent_mask = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
    XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_VISIBILITY_CHANGE |XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
};


void (*events[LAST_EVENT])() = {
    [XCB_KEY_PRESS] = onKeyPress,
    [XCB_CONFIGURE_NOTIFY] = onConfigureEvent,

    [PROCESS_ARGS] = parse_options,
    [POST_XCONNECTION] = initlizeBindings,
    [POST_EVENT] = render,
};

Binding bindings[] = {
    {0, XK_q, exit, 1},
    {0, XK_f, cycleScaleMode, 1},
    {ShiftMask, XK_f, cycleScaleMode, -1},
    {0, XK_semicolon, cycleAlignment, 1},
    {ShiftMask, XK_semicolon, cycleAlignment, -1},
    {0, XK_minus, zoom,        -2},
    {ShiftMask, XK_plus, zoom, +2},
    {0, XK_equal, zoom,        0},
    {0, XK_d, toggle_multi_page,        2},

    {0, XK_j, next_image,  +1},
    {0, XK_k, next_image,  -1},
    {0, XK_space, next_page,  +1},
    {ShiftMask, XK_space, next_page,  -1},
    {0}
};
