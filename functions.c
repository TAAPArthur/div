
#include "div.h"
#include "image.h"
#include "functions.h"

void open_images() {
    for (int i = 0; i < state.num_active_images; i++) {
        img_load(image_holders + i, getFilePath(state.file_index + i));
    }
}

void next_image(int arg) {
    if(state.file_index + arg < state.num_files && state.file_index + arg + state.num_active_images > 0) {
        state.file_index += arg;
        open_images();
    }
}

void next_page(int arg) {
    next_image(arg * state.num_active_images);
}

void jump_start() {
    state.file_index = 0;
    open_images();
}

void jump_end() {
    state.file_index = state.file_index - state.num_active_images;
    open_images();
}

void toggle_multi_page(int arg) {
    state.num_active_images = state.num_active_images == arg ? 1 : arg;
}

void cycleAlignment(int delta) {
    state.align_mode_x = CYCLE(state.align_mode_x, delta, LAST_ALIGN_MODE);
    state.align_mode_y = CYCLE(state.align_mode_y, delta, LAST_ALIGN_MODE);
}

void setAlignment(AlignMode mode) {
    state.align_mode_x = mode;
    state.align_mode_y = mode;
}

void setScaleMode(ScaleMode mode) {
    state.scale_mode = mode;
}

void cycleScaleMode(int delta) {
    state.scale_mode = CYCLE(state.scale_mode, delta, LAST_SCALE_MODE);
}

void zoom(int delta) {
    if(delta > 0)
        state.zoom *= delta;
    else if(delta < 0)
        state.zoom /= -delta;
    else
        state.zoom = 1;
}
