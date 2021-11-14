

#include "div.h"
float get_img_zoom(uint32_t image_width, uint32_t image_height, float zoom, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode);

float get_zoom(uint32_t image_width, uint32_t image_height, float holder_zoom, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode) ;

float get_effective_dim(uint32_t image_width, uint32_t image_height, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode, bool dim) ;

uint32_t adjustAlignment(AlignMode mode, uint32_t used_value, uint32_t max_value) ;

void scale(const char* const buf, uint16_t original_width, uint16_t original_height, unsigned int width, unsigned int height, unsigned int bytesperline, char *newbuf, int H);

void img_render(ImageInfo*holder, int num, uint32_t wid, uint32_t win_width, uint32_t win_height);
