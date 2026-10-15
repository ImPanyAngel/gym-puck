#pragma once

// Both functions take the LVGL lock themselves, so call them from
// setup() or loop(), never from inside an LVGL event callback.
void ui_init(void);
void ui_show_button_press(const char *name);
