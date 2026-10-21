/*
 * Portrait status screen for the vertically mounted Sofle OLED.
 */

#include <zmk/display/status_screen.h>

#include "util.h"

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, LVGL_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    sofle_status_init(screen);
    return screen;
}
