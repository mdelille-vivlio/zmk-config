/*
 * Right (peripheral) half: battery level and the link to the left half.
 * This half never sees keycodes, so it has no last-key block.
 *
 *   block 0: battery level
 *   block 1: link to the left half (check or cross)
 */

#include <stdio.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/split/bluetooth/peripheral.h>
#include <zmk/usb.h>

#include "util.h"

enum { BLOCK_BATTERY, BLOCK_LINK };

struct battery_status_state {
    uint8_t level;
    bool usb_present;
};

struct peripheral_status_state {
    bool connected;
};

#if IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)
static void battery_status_update_cb(struct battery_status_state state) {
    lv_obj_t *canvas = sofle_block_begin(BLOCK_BATTERY);
    char text[16] = {};

    if (state.usb_present) {
        snprintf(text, sizeof(text), "%d%s", state.level, LV_SYMBOL_CHARGE);
    } else {
        snprintf(text, sizeof(text), "%d%%", state.level);
    }
    sofle_draw_text_centered(canvas, sofle_fit_font(text), text);

    sofle_block_end(BLOCK_BATTERY);
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

    return (struct battery_status_state){
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif
#endif

static void peripheral_status_update_cb(struct peripheral_status_state state) {
    lv_obj_t *canvas = sofle_block_begin(BLOCK_LINK);
    sofle_draw_text_centered(canvas, &lv_font_montserrat_14,
                             state.connected ? LV_SYMBOL_OK : LV_SYMBOL_CLOSE);
    sofle_block_end(BLOCK_LINK);
}

static struct peripheral_status_state peripheral_status_get_state(const zmk_event_t *_eh) {
    return (struct peripheral_status_state){.connected = zmk_split_bt_peripheral_is_connected()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_status, struct peripheral_status_state,
                            peripheral_status_update_cb, peripheral_status_get_state)
ZMK_SUBSCRIPTION(widget_peripheral_status, zmk_split_peripheral_status_changed);

void sofle_status_init(lv_obj_t *parent) {
    sofle_blocks_init(parent);

#if IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)
    widget_battery_status_init();
#endif
    widget_peripheral_status_init();
}
