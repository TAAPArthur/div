#include "div.h"
#include "image_view.h"

float get_img_zoom(uint32_t image_width, uint32_t image_height, float zoom, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode) {
    float zw, zh;

    zw = (float) image_width / (float) win_width;
    zh = (float) image_height / (float) win_height;

    switch (scale_mode) {
        case SCALE_MIN:
            return MAX(zw, zh);
        case SCALE_MAX:
            return MIN(zw, zh);
        case SCALE_WIDTH:
            return zw;
        case SCALE_HEIGHT:
            return zh;
        default:
            return zoom ? zoom : 1;
    }
}

float get_zoom(uint32_t image_width, uint32_t image_height, float holder_zoom, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode) {
    float zoom = get_img_zoom(image_width, image_height, holder_zoom, win_width, win_height, scale_mode);
    return (state.zoom ? state.zoom : 1) * (zoom ? zoom : 1);
}

float get_effective_dim(uint32_t image_width, uint32_t image_height, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode, bool dim) {
    float zw, zh;

    zw = (float) image_width / (float) win_width;
    zh = (float) image_height / (float) win_height;
    if(scale_mode == SCALE_MIN || scale_mode == SCALE_MAX) {
        scale_mode = ((zw > zh) == (scale_mode == SCALE_MIN)) ? SCALE_WIDTH : SCALE_HEIGHT;
    }
    switch (scale_mode) {
        case SCALE_WIDTH:
            return !dim ? win_width : image_height / zw;
        case SCALE_HEIGHT:
            return dim ? win_height : image_width / zh;
        case SCALE_NORMAL:
            return dim ? image_height : image_width;
        case SCALE_FILL:
        default:
            return dim ? win_height : win_width;
    }
}

uint32_t adjustAlignment(AlignMode mode, uint32_t used_value, uint32_t max_value) {
    if(used_value > max_value) {
        return 0;
    }
    switch (mode) {
        default:
        case ALIGN_CENTER:
            return (max_value - used_value) / 2;
        case ALIGN_BOTTOM_RIGHT:
            return max_value - used_value;
        case ALIGN_TOP_LEFT:
            return 0;
    }
}

void nearestNeighbourScale(const char* buf, uint32_t original_width, uint32_t original_height, char* out_buf, uint32_t width, uint32_t height, int num_channels) {
    unsigned int bytesperline = width * num_channels;
    unsigned int jdy = bytesperline / num_channels - width;
    unsigned int dx = (original_width << 10) / width;
    for (unsigned int y = 0; y < height; y++) {
        unsigned int bufx = original_width / width;
        const unsigned char *ibuf = &buf[y * original_height / height * original_width * num_channels ];
        for (unsigned int x = 0; x < width; x++) {
            *out_buf++ = ibuf[(bufx >> 10) * num_channels + 0];
            *out_buf++ = ibuf[(bufx >> 10) * num_channels + 1];
            *out_buf++ = ibuf[(bufx >> 10) * num_channels + 2];
            out_buf++;
            bufx += dx;
        }
        out_buf += jdy;
    }
}
