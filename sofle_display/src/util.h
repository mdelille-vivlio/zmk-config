/*
 * Portrait status screen for the vertically mounted Sofle OLED.
 *
 * The 128x32 display is split into four 32x32 blocks. Each block is drawn
 * upright into its own canvas, then rotated in place (the same technique as
 * ZMK's nice!view status screen), so the screen reads top to bottom.
 */

#pragma once

#include <lvgl.h>
#include <zephyr/sys/util.h>

#define CANVAS_SIZE 32
#define BLOCK_COUNT 4

// The SSD1306 lights a pixel for LVGL "black" (1-bit mono format), so the
// colors are swapped here: "white" is an unlit pixel, i.e. a dark background.
#define LVGL_BACKGROUND                                                                            \
    (IS_ENABLED(CONFIG_SOFLE_DISPLAY_INVERTED) ? lv_color_black() : lv_color_white())
#define LVGL_FOREGROUND                                                                            \
    (IS_ENABLED(CONFIG_SOFLE_DISPLAY_INVERTED) ? lv_color_white() : lv_color_black())

// Creates the block canvases on the screen. Block 0 is the top of the portrait screen.
void sofle_blocks_init(lv_obj_t *parent);

// Clears a block and returns its canvas, ready to draw in portrait coordinates.
lv_obj_t *sofle_block_begin(int block);

// Rotates the block to the display orientation. Call after drawing.
void sofle_block_end(int block);

// Picks Montserrat 14 if the text fits the block width, else Montserrat 10.
const lv_font_t *sofle_fit_font(const char *text);

// Draws text centered horizontally in a box of the given width.
void sofle_draw_text(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t w,
                     const lv_font_t *font, lv_color_t color, const char *text);

// Draws text centered both ways in the block.
void sofle_draw_text_centered(lv_obj_t *canvas, const lv_font_t *font, const char *text);

void sofle_fill_rect(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                     lv_color_t color);

// Implemented by status.c (left half) or peripheral.c (right half).
void sofle_status_init(lv_obj_t *parent);
