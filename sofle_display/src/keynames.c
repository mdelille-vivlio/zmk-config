/*
 * Short names for HID usages, sized for a 32 px wide block.
 */

#include <stdio.h>

#include <lvgl.h>
#include <dt-bindings/zmk/hid_usage.h>
#include <dt-bindings/zmk/hid_usage_pages.h>

#include "keynames.h"

// Keyboard page usages 0x28 (Enter) to 0x52 (Up).
static const char *const keyboard_names[] = {
    LV_SYMBOL_NEW_LINE, "ESC", LV_SYMBOL_BACKSPACE, "TAB", "SPC", "-", "=", "[", "]", "\\", "#",
    ";", "'", "`", ",", ".", "/", "CAPS",
    // 0x3A-0x45: F1-F12 are generated below.
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "PSCR", "SCRL", "PAUS", "INS", "HOME", "PGUP", "DEL", "END", "PGDN", LV_SYMBOL_RIGHT,
    LV_SYMBOL_LEFT, LV_SYMBOL_DOWN, LV_SYMBOL_UP};

static const char *keyboard_name(uint32_t id) {
    if (id >= 0x28 && id <= 0x52) {
        return keyboard_names[id - 0x28];
    }
    switch (id) {
    case 0x65:
        return "MENU";
    case 0x7A:
        return "UNDO";
    case 0x7B:
        return LV_SYMBOL_CUT;
    case 0x7C:
        return LV_SYMBOL_COPY;
    case 0x7D:
        return LV_SYMBOL_PASTE;
    default:
        return NULL;
    }
}

static const char *consumer_name(uint32_t id) {
    switch (id) {
    case 0xB5:
        return LV_SYMBOL_NEXT;
    case 0xB6:
        return LV_SYMBOL_PREV;
    case 0xCD:
        return LV_SYMBOL_PLAY;
    case 0xE2:
        return LV_SYMBOL_MUTE;
    case 0xE9:
        return LV_SYMBOL_VOLUME_MAX;
    case 0xEA:
        return LV_SYMBOL_VOLUME_MID;
    default:
        return NULL;
    }
}

void sofle_key_name(uint16_t usage_page, uint32_t keycode, char *buf, size_t len) {
    const char *name = NULL;

    if (usage_page == HID_USAGE_KEY) {
        if (keycode >= 0x04 && keycode <= 0x1D) {
            snprintf(buf, len, "%c", 'A' + (int)(keycode - 0x04));
            return;
        }
        if (keycode >= 0x1E && keycode <= 0x27) {
            snprintf(buf, len, "%d", (int)((keycode - 0x1E + 1) % 10));
            return;
        }
        if (keycode >= 0x3A && keycode <= 0x45) {
            snprintf(buf, len, "F%d", (int)(keycode - 0x3A + 1));
            return;
        }
        if (keycode >= 0x68 && keycode <= 0x73) {
            snprintf(buf, len, "F%d", (int)(keycode - 0x68 + 13));
            return;
        }
        name = keyboard_name(keycode);
    } else if (usage_page == HID_USAGE_CONSUMER) {
        name = consumer_name(keycode);
    }

    if (name != NULL) {
        snprintf(buf, len, "%s", name);
    } else {
        snprintf(buf, len, "%02X", (unsigned int)(keycode & 0xFFFF));
    }
}
