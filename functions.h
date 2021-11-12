#ifndef DIV_FUNC
#define DIV_FUNC
#include "div.h"

#define CYCLE(X, DELTA, MAX_VALUE) (((X + DELTA) % MAX_VALUE + MAX_VALUE) %MAX_VALUE)

void cycleAlignment(int delta) ;
void setAlignment(AlignMode mode) ;
void setScaleMode(ScaleMode mode) ;
void cycleScaleMode(int delta) ;


void next_page(int arg);
void open_images();
void next_image(int arg) ;

void toggle_multi_page(int arg);
void toggle_right_to_left();
void toggle_grid(int arg);

void jump_start() ;

void jump_end() ;

void zoom(int delta) ;
#endif
