#include "div.h"
#include <xcb/xcb_image.h>
#include <xcb/xcb.h>
#include "image.h"

#include <stdbool.h>

int img_load(ImageHolder* holder, const char* path)
{
    if(holder->image_data) {
        imlib_context_set_image(holder->image_data);
        imlib_free_image();
    }

	if ((holder->image_data = path ? imlib_load_image(path) : NULL) == NULL) {
		return 0;
    }
    holder->path = path;

    imlib_context_set_image(holder->image_data);

    holder->image_width = imlib_image_get_width();
    holder->image_height = imlib_image_get_height();
	return 1;
}

float get_img_zoom(ImageHolder*holder, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode) {
	float zw, zh;

	zw = (float) holder->image_width / (float) win_width;
	zh = (float) holder->image_height / (float) win_height;

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
            return holder->zoom ? holder->zoom : 1;
	}
}

float get_zoom(ImageHolder*holder, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode) {
    float zoom = get_img_zoom(holder, win_width, win_height, scale_mode);
    return (state.zoom ? state.zoom : 1) * (zoom ? zoom : 1);
}

float get_effective_dim(ImageHolder*holder, uint32_t win_width, uint32_t win_height, ScaleMode scale_mode, bool dim) {
	float zw, zh;

	zw = (float) holder->image_width / (float) win_width;
	zh = (float) holder->image_height / (float) win_height;
    if(scale_mode == SCALE_MIN || scale_mode == SCALE_MAX) {
        scale_mode = ((zw > zh) == (scale_mode == SCALE_MIN)) ? SCALE_WIDTH : SCALE_HEIGHT;
    }
	switch (scale_mode) {
		case SCALE_WIDTH:
            if(!dim)
                return win_width;
            else
                return holder->image_height / zw;
		case SCALE_HEIGHT:
            if(dim)
                return win_height;
            else
                return holder->image_width / zh;
        default:
            if(dim)
                return win_height;
            else
                return win_width;
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


/* scales imgbuf data to newbuf (ximg->data), nearest neighbour. */
void scale(const char* const buf, uint16_t original_width, uint16_t original_height, unsigned int width, unsigned int height, unsigned int bytesperline, char *newbuf, int H)
{
	const unsigned char *ibuf;
	unsigned int jdy, dx, bufx, x, y;
	float a = 0.0f;
    int c = 4;

	jdy = bytesperline / c - width;
	dx = (original_width << 10) / width;
	for (y = 0; y < H; y++) {
		bufx = original_width / width;
		ibuf = &buf[y * original_height / height * original_width * c];

		for (x = 0; x < width; x++) {

			a = c==4?(ibuf[(bufx >> 10)*c+3]) / 255.0f:1;
			*newbuf++ = (ibuf[(bufx >> 10)*c+0] * a);
			*newbuf++ = (ibuf[(bufx >> 10)*c+1] * a);
			*newbuf++ = (ibuf[(bufx >> 10)*c+2] * a);
			newbuf++;
			bufx += dx;
		}
		newbuf += jdy;
	}
}

void img_render(ImageHolder*holder, int num, uint32_t wid, uint32_t win_width, uint32_t win_height) {
	uint32_t dw, dh;
    uint32_t total_image_width = 0, total_image_height = 0;

    dw = (win_width - state.padding_x *2) / getCols();
    dh = (win_height- state.padding_y *2) / getRows();

    for (int i = 0; i < getCols() && i < num; i++) {
        total_image_width += get_effective_dim(&holder[i], dw, dh, state.scale_mode, 0) + holder[i].padding_x;
    }

    for (int i = 0; i < num; i+=getCols()) {
        total_image_height += get_effective_dim(&holder[i], dw, dh, state.scale_mode, 1) + holder[i].padding_y;
    }

    int startingX = state.padding_x + adjustAlignment(state.align_mode_x, total_image_width, win_width);
    int y = state.padding_y + adjustAlignment(state.align_mode_y, total_image_height, win_height);

    uint32_t effective_width;
    uint32_t effective_height;
    for (int r = 0, i = 0; r < getRows(); r++) {
        int x = state.right_to_left ? win_width - startingX : startingX;
        for (int c = 0; c < getCols() && i < num; c++, i++) {
            if(!holder[i].image_data)
                continue;
            imlib_context_set_image(holder[i].image_data);

            float zoom = state.zoom ? state.zoom : 1;
            effective_width = get_effective_dim(&holder[i], dw, dh, state.scale_mode, 0) ;
            effective_height = get_effective_dim(&holder[i], dw, dh, state.scale_mode, 1);

            //imlib_render_image_part_on_drawable_at_size(holder[i].offset_x, holder[i].offset_y,
            //        holder[i].image_width/zoom , holder[i].image_height/zoom, x - (state.right_to_left? effective_width: 0), y, effective_width, effective_height);

           extern int depth;
           const void*data=imlib_image_get_data_for_reading_only();

           extern xcb_connection_t* dis;

           xcb_image_t *image = xcb_image_create_native(dis,effective_width*zoom ,effective_height*zoom ,XCB_IMAGE_FORMAT_Z_PIXMAP ,depth,NULL, 0,NULL);

           scale(data, holder[i].image_width, holder[i].image_height, effective_width*zoom , effective_height*zoom, image->stride, image->data, MIN(effective_height,  effective_height*zoom));

           if(zoom > 1) {
               xcb_image_t *sub_image = xcb_image_subimage(image,holder[i].offset_x, holder[i].offset_y, effective_width ,effective_height,NULL,0,NULL);
              xcb_image_destroy(image);
              image = sub_image;
           }
            extern xcb_gcontext_t gc;
            xcb_image_put(dis, wid, gc, image , x - (state.right_to_left? effective_width: 0), y, 0);
            xcb_image_destroy(image);

            x+=(effective_width + holder[i].padding_x) * (state.right_to_left?-1:1);
        }
        y += effective_height;
    }
}
