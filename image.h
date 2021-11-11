#ifndef DIV_IMAGE
#define DIV_IMAGE

#include <Imlib2.h>
#include "div.h"
int img_load(ImageHolder* holder, const char* path);

void img_render(ImageHolder*holder, int num, uint32_t wid, uint32_t win_width, uint32_t win_height) ;
#endif
