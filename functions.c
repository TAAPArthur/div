#include "div.h"
#include "functions.h"

#define TOGGLE(A,B, D) A = (A==B ? D : B)

void open_images() {
    state.render_count++;
    RUN_EVENT(OPEN_IMAGES);
}

void next_image(int arg) {
    if(state.wrap) {
        state.file_index = (state.num_files + (state.file_index + arg) % state.num_files) % state.num_files;
        open_images();
    } else if(state.file_index + arg < state.num_files && state.file_index + arg + getNumActiveImages() > 0) {
        state.file_index += arg;
        open_images();
    }
}

void next_page(int arg) {
    reset_pan();
    next_image(arg * getNumActiveImages());
}

void jump_start() {
    state.file_index = 0;
    open_images();
}

void jump_end() {
    state.file_index = state.num_files - getNumActiveImages();
    open_images();
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

float ZOOM_STEP = 1.2599210498948732; /* 2^(1/3) */
void zoom(int delta) {
    if(delta > 0)
        state.zoom *= ZOOM_STEP;
    else if(delta < 0)
        state.zoom /= ZOOM_STEP;
    else
        state.zoom = 1;
    state.zoom = MAX(1/64.0, MIN(state.zoom, 16));
}

void reset_pan() {
    state.start_x = state.start_y = 0;
}

void pan_x(int delta) {
    state.start_x += delta;
}

void pan_y(int delta) {
    state.start_y += delta;
}

void toggle_right_to_left(){
    state.right_to_left = !state.right_to_left;
}

void toggle_grid(int arg){
    TOGGLE(state.rows,arg, 0);
    TOGGLE(state.cols,arg, 0);
}

void toggle_column(int arg) { TOGGLE(state.cols, arg, 1); }

void toggle_row(int arg) { TOGGLE(state.cols, arg, 1); }
