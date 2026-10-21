/*
 * Portrait status screen for the vertically mounted Sofle OLED.
 */

#include <string.h>
#include <zephyr/kernel.h>

#include "util.h"

static lv_obj_t *canvases[BLOCK_COUNT];
static lv_color_t cbufs[BLOCK_COUNT][CANVAS_SIZE * CANVAS_SIZE];

// Block 0 is the top of the portrait screen. With a 90 degree rotation it ends up on the right
// of the landscape display, with 270 degrees on the left.
static lv_coord_t block_x(int block) {
    if (CONFIG_SOFLE_DISPLAY_ROTATION == 2700) {
        return block * CANVAS_SIZE;
    }
    return (BLOCK_COUNT - 1 - block) * CANVAS_SIZE;
}

void sofle_blocks_init(lv_obj_t *parent) {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        canvases[i] = lv_canvas_create(parent);
        lv_canvas_set_buffer(canvases[i], cbufs[i], CANVAS_SIZE, CANVAS_SIZE,
                             LV_IMG_CF_TRUE_COLOR);
        lv_obj_set_pos(canvases[i], block_x(i), 0);
        sofle_block_begin(i);
        sofle_block_end(i);
    }
}

lv_obj_t *sofle_block_begin(int block) {
    lv_obj_t *canvas = canvases[block];
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);
    return canvas;
}

void sofle_block_end(int block) {
    static lv_color_t cbuf_tmp[CANVAS_SIZE * CANVAS_SIZE];
    memcpy(cbuf_tmp, cbufs[block], sizeof(cbuf_tmp));

    lv_img_dsc_t img = {
        .data = (void *)cbuf_tmp,
        .header.cf = LV_IMG_CF_TRUE_COLOR,
        .header.w = CANVAS_SIZE,
        .header.h = CANVAS_SIZE,
    };

    lv_obj_t *canvas = canvases[block];
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);
    lv_canvas_transform(canvas, &img, CONFIG_SOFLE_DISPLAY_ROTATION, LV_IMG_ZOOM_NONE,
                        CONFIG_SOFLE_DISPLAY_ROTATION == 2700 ? 0 : -1,
                        CONFIG_SOFLE_DISPLAY_ROTATION == 2700 ? -1 : 0, CANVAS_SIZE / 2,
                        CANVAS_SIZE / 2, true);
}

const lv_font_t *sofle_fit_font(const char *text) {
    lv_point_t size;
    lv_txt_get_size(&size, text, &lv_font_montserrat_14, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    return size.x <= CANVAS_SIZE ? &lv_font_montserrat_14 : &lv_font_montserrat_10;
}

void sofle_draw_text(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t w,
                     const lv_font_t *font, lv_color_t color, const char *text) {
    lv_draw_label_dsc_t dsc;
    lv_draw_label_dsc_init(&dsc);
    dsc.color = color;
    dsc.font = font;
    dsc.align = LV_TEXT_ALIGN_CENTER;
    lv_canvas_draw_text(canvas, x, y, w, &dsc, text);
}

void sofle_draw_text_centered(lv_obj_t *canvas, const lv_font_t *font, const char *text) {
    lv_coord_t y = (CANVAS_SIZE - lv_font_get_line_height(font)) / 2;
    sofle_draw_text(canvas, 0, y, CANVAS_SIZE, font, LVGL_FOREGROUND, text);
}

void sofle_fill_rect(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                     lv_color_t color) {
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color;
    lv_canvas_draw_rect(canvas, x, y, w, h, &dsc);
}
