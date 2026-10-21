/*
 * Left (central) half: output and battery, layer name, held modifiers and the last key.
 *
 *   block 0: USB or Bluetooth profile, battery level
 *   block 1: layer name
 *   block 2: held modifiers (C S / A G)
 *   block 3: last key
 */

#include <stdio.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/hid.h>
#include <zmk/keys.h>
#include <zmk/keymap.h>
#include <zmk/usb.h>

#include "keynames.h"
#include "util.h"

enum { BLOCK_OUTPUT, BLOCK_LAYER, BLOCK_MODS, BLOCK_KEY };

struct output_status_state {
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
};

struct battery_status_state {
    uint8_t level;
    bool usb_present;
};

struct layer_status_state {
    zmk_keymap_layer_index_t index;
    const char *label;
};

struct key_status_state {
    uint8_t mods; // bits: ctrl, shift, alt, gui (left and right merged)
    char key[8];
};

static struct output_status_state output_state;
static struct battery_status_state battery_state;

static void draw_output_battery(void) {
    lv_obj_t *canvas = sofle_block_begin(BLOCK_OUTPUT);
    char text[16] = {};

    if (output_state.selected_endpoint.transport == ZMK_TRANSPORT_USB) {
        snprintf(text, sizeof(text), "%s", LV_SYMBOL_USB);
    } else {
        snprintf(text, sizeof(text), "%s%d", LV_SYMBOL_BLUETOOTH,
                 output_state.active_profile_index + 1);
    }

    // A disconnected Bluetooth profile is drawn inverted.
    bool inverted = output_state.selected_endpoint.transport == ZMK_TRANSPORT_BLE &&
                    !output_state.active_profile_connected;
    if (inverted) {
        sofle_fill_rect(canvas, 0, 0, CANVAS_SIZE, 17, LVGL_FOREGROUND);
    }
    sofle_draw_text(canvas, 0, 1, CANVAS_SIZE, &lv_font_montserrat_14,
                    inverted ? LVGL_BACKGROUND : LVGL_FOREGROUND, text);

#if IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)
    if (battery_state.usb_present) {
        snprintf(text, sizeof(text), "%d%s", battery_state.level, LV_SYMBOL_CHARGE);
    } else {
        snprintf(text, sizeof(text), "%d%%", battery_state.level);
    }
    sofle_draw_text(canvas, 0, 20, CANVAS_SIZE, &lv_font_montserrat_10, LVGL_FOREGROUND, text);
#endif

    sofle_block_end(BLOCK_OUTPUT);
}

static void output_status_update_cb(struct output_status_state state) {
    output_state = state;
    draw_output_battery();
}

static struct output_status_state output_status_get_state(const zmk_event_t *_eh) {
    return (struct output_status_state){
        .selected_endpoint = zmk_endpoints_selected(),
#if IS_ENABLED(CONFIG_ZMK_BLE)
        .active_profile_index = zmk_ble_active_profile_index(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
#endif
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if IS_ENABLED(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

#if IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)
static void battery_status_update_cb(struct battery_status_state state) {
    battery_state = state;
    draw_output_battery();
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

static void layer_status_update_cb(struct layer_status_state state) {
    lv_obj_t *canvas = sofle_block_begin(BLOCK_LAYER);
    char text[16] = {};

    if (state.label == NULL || state.label[0] == '\0') {
        snprintf(text, sizeof(text), "L%d", state.index);
    } else {
        snprintf(text, sizeof(text), "%s", state.label);
    }
    sofle_draw_text_centered(canvas, sofle_fit_font(text), text);

    sofle_block_end(BLOCK_LAYER);
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){
        .index = index, .label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index))};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)
ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

static void key_status_update_cb(struct key_status_state state) {
    // Held modifiers in a fixed 2x2 grid, so letters don't move around.
    static const char *const mod_letters[] = {"C", "S", "A", "G"};
    lv_obj_t *canvas = sofle_block_begin(BLOCK_MODS);
    for (int i = 0; i < 4; i++) {
        if (state.mods & BIT(i)) {
            sofle_draw_text(canvas, (i % 2) * 16, (i / 2) * 16, 16, &lv_font_montserrat_14,
                            LVGL_FOREGROUND, mod_letters[i]);
        }
    }
    sofle_block_end(BLOCK_MODS);

    canvas = sofle_block_begin(BLOCK_KEY);
    if (state.key[0] != '\0') {
        sofle_draw_text_centered(canvas, sofle_fit_font(state.key), state.key);
    }
    sofle_block_end(BLOCK_KEY);
}

static struct key_status_state key_status_get_state(const zmk_event_t *eh) {
    // Releases and modifier-only presses keep the last key on screen.
    static struct key_status_state last;
    const struct zmk_keycode_state_changed *ev =
        eh != NULL ? as_zmk_keycode_state_changed(eh) : NULL;

    if (ev != NULL && ev->state && !is_mod(ev->usage_page, ev->keycode)) {
        zmk_mod_flags_t mods = zmk_hid_get_explicit_mods() | ev->implicit_modifiers;
        last.mods = (mods | (mods >> 4)) & 0x0F;
        sofle_key_name(ev->usage_page, ev->keycode, last.key, sizeof(last.key));
    }
    return last;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_key_status, struct key_status_state, key_status_update_cb,
                            key_status_get_state)
ZMK_SUBSCRIPTION(widget_key_status, zmk_keycode_state_changed);

void sofle_status_init(lv_obj_t *parent) {
    sofle_blocks_init(parent);

    widget_output_status_init();
#if IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)
    widget_battery_status_init();
#endif
    widget_layer_status_init();
    widget_key_status_init();
}
