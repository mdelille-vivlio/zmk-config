/*
 * Bootloader key that leaves a "flash me" screen on the OLED.
 *
 * The UF2 bootloader never touches the display, and the SSD1306 keeps its last
 * image through a reboot. So the screen is drawn and flushed first, then the
 * half reboots into the bootloader, and the image stays until the new firmware
 * redraws it.
 */

#define DT_DRV_COMPAT sofle_behavior_boot_screen

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

#include <drivers/behavior.h>
#include <dt-bindings/zmk/reset.h>

#include <zmk/behavior.h>
#include <zmk/display.h>

#include "util.h"

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static void boot_screen_work_cb(struct k_work *work) {
    lv_obj_t *canvas = sofle_block_begin(0);
    sofle_draw_text_centered(canvas, &lv_font_montserrat_14, LV_SYMBOL_DOWNLOAD);
    sofle_block_end(0);

    canvas = sofle_block_begin(1);
    sofle_fill_rect(canvas, 0, 0, CANVAS_SIZE, CANVAS_SIZE, LVGL_FOREGROUND);
    sofle_draw_text(canvas, 0, (CANVAS_SIZE - lv_font_get_line_height(&lv_font_montserrat_10)) / 2,
                    CANVAS_SIZE, &lv_font_montserrat_10, LVGL_BACKGROUND, "BOOT");
    sofle_block_end(1);

    canvas = sofle_block_begin(2);
    sofle_draw_text_centered(canvas, &lv_font_montserrat_10, "UF2");
    sofle_block_end(2);

    canvas = sofle_block_begin(3);
    sofle_draw_text_centered(canvas, &lv_font_montserrat_14, LV_SYMBOL_USB);
    sofle_block_end(3);

    // Wake the display if it went idle, and push the frame out before rebooting.
    display_blanking_off(DEVICE_DT_GET(DT_CHOSEN(zephyr_display)));
    lv_refr_now(NULL);

    sys_reboot(RST_UF2);
}

K_WORK_DEFINE(boot_screen_work, boot_screen_work_cb);

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    if (zmk_display_is_initialized()) {
        k_work_submit_to_queue(zmk_display_work_q(), &boot_screen_work);
    } else {
        sys_reboot(RST_UF2);
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_boot_screen_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    // Runs on the half where the key is pressed, like ZMK's &bootloader.
    .locality = BEHAVIOR_LOCALITY_EVENT_SOURCE,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define BOOT_SCREEN_INST(n)                                                                        \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_boot_screen_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BOOT_SCREEN_INST)

#endif
