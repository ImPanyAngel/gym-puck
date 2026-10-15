#pragma once

#include <stdint.h>

// Shared colours for every screen, as 0xRRGGBB. Use with lv_color_hex().
constexpr uint32_t UI_COLOR_BACKGROUND = 0x000000;   // black: AMOLED pixels off
constexpr uint32_t UI_COLOR_PRIMARY    = 0xC0C0C0;   // light grey: progress, text, accents
constexpr uint32_t UI_COLOR_SECONDARY  = 0x808080;   // mid grey: less important text, e.g. the artist
constexpr uint32_t UI_COLOR_TRACK      = 0x303030;   // dark grey: unfilled part of the arc
