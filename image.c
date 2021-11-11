#include "div.h"
#include "image.h"

#include <stdbool.h>
#include <stdio.h>

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
        printf("Warning to big %d %d\n", used_value, max_value);
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

void img_render(ImageHolder*holder, int num, uint32_t wid, uint32_t win_width, uint32_t win_height) {
	uint32_t dw, dh;
    uint32_t total_image_width = 0, total_image_height = 0;
    printf("WID %d %d num R %d C %d %d\n",wid, num, getRows(), getCols(), state.num_active_images);

    dw = (win_width - state.padding_x *2) / getCols();
    dh = (win_height- state.padding_y *2) / getRows();

    for (int i = 0; i < getCols(); i++) {
            total_image_width += get_effective_dim(&holder[i], dw, dh, state.scale_mode, 0) + holder[i].padding_x;
    }

    for (int i = 0; i < getRows(); i+=getCols()) {
            total_image_height += get_effective_dim(&holder[i], dw, dh, state.scale_mode, 1) + holder[i].padding_y;
    }

    int startingX = state.padding_x + adjustAlignment(state.align_mode_x, total_image_width, win_width);
    int y = state.padding_y + adjustAlignment(state.align_mode_y, total_image_height, win_height);

    printf("X %d Y %d %d %d\n", startingX , y, total_image_width ,total_image_height );

    imlib_context_set_drawable(wid);
    //imlib_image_fill_rectangle(0, 0, win_width, win_height);


    uint32_t effective_width;
    uint32_t effective_height;
    for (int r = 0, i = 0; r < getRows(); r++) {
        int x = state.right_to_left ? win_width - startingX : startingX;
        for (int c = 0; c < getCols() && i < num; c++, i++) {
            if(!holder[i].image_data)
                continue;
            imlib_context_set_image(holder[i].image_data);

            effective_width = get_effective_dim(&holder[i], dw, dh, state.scale_mode, 0);
            effective_height = get_effective_dim(&holder[i], dw, dh, state.scale_mode, 1);
            float zoom = state.zoom ? state.zoom : 1;
            printf("%s, %d Window %d %d Image %d %d Effective %d %d %f %d\n",holder[i].path, i, win_width, win_height, holder[i].image_width, holder[i].image_height, effective_width ,effective_height , zoom, state.scale_mode);

            imlib_render_image_part_on_drawable_at_size(holder[i].offset_x, holder[i].offset_y,
                    holder[i].image_width/zoom , holder[i].image_height/zoom, x - (state.right_to_left? effective_width: 0), y, effective_width, effective_height);
            x+=(effective_width + holder[i].padding_x) * (state.right_to_left?-1:1);
        }
        y += effective_height;
    }
}
