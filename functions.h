#ifndef DIV_FUNC
#define DIV_FUNC
#include "div.h"

#define CYCLE(X, DELTA, MAX_VALUE) (((X + DELTA) % MAX_VALUE + MAX_VALUE) %MAX_VALUE)

void cycleAlignment(int delta) ;
void setAlignment(AlignMode mode) ;
void setScaleMode(ScaleMode mode) ;
void cycleScaleMode(int delta) ;


/**
 * Resets any panning and then calls next_image arg * getNumActiveImages()
 * Calls next_image arg times
 */
void next_page(int arg);
/**
 * Modifies the file_index by arg. If the wrap flag is set, this function handles wrapping
 */
void next_image(int arg) ;
/*
 * Trigger the OPEN_IMAGES event
 */
void open_images();

void toggle_right_to_left();
void toggle_grid(int arg);
void toggle_column(int arg);
void toggle_row(int arg);

void jump_start() ;

void jump_end() ;

void zoom(int delta);
void pan_x(int delta);
void pan_y(int delta);
void reset_pan();
#endif
