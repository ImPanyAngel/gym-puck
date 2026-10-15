#pragma once

// These take the LVGL lock themselves, so call them from setup() or loop(),
// never from inside an LVGL event callback.
void ui_init(void);
void ui_next_screen(void);
