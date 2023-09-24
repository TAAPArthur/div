#ifndef DIV_X
#define DIV_X

#include <stdbool.h>
#include <stdint.h>

uint32_t setupXConnection();

void clear_window(uint32_t wid, uint16_t width, uint16_t height);
void clear_drawable(uint32_t wid, uint16_t width, uint16_t height);
void processXEvents() ;
bool processQueuedXEvents();

void initlizeBindings();
void onKeyPress();
void onConfigureEvent();

void setWindowTitle(const char* str);

void changeBackground(uint32_t color);

void invertBackground();


void closeXConnection();
int isXConnectionOpen();

void setWindowProperties(uint32_t wid);

void flush();
#endif
