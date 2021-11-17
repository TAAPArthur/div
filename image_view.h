#include "div.h"
float get_img_zoom(uint32_t image_width, uint32_t image_height, float zoom, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode);

float get_zoom(uint32_t image_width, uint32_t image_height, float holder_zoom, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode) ;

float get_effective_dim(uint32_t image_width, uint32_t image_height, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode, bool dim) ;

uint32_t adjustAlignment(AlignMode mode, uint32_t used_value, uint32_t max_value) ;

void img_render(ImageInfo*holder, int num, uint32_t wid, uint32_t win_width, uint32_t win_height);

void nearestNeighbourScale(const char* buf, uint32_t original_width, uint32_t original_height, char* out_buf, uint32_t width, uint32_t height, int num_channels);
