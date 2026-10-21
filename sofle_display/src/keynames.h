#pragma once

#include <stddef.h>
#include <stdint.h>

// Writes a short display name for a HID usage: an LVGL symbol or a few characters.
void sofle_key_name(uint16_t usage_page, uint32_t keycode, char *buf, size_t len);
