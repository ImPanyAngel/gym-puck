#pragma once

// These take the LVGL lock themselves, so call them from setup() or loop(),
// never from inside an LVGL event callback.
void ui_init(void);
void ui_next_screen(void);   // does nothing while the screen is off
void ui_toggle_screen(void); // off: panel asleep, touch ignored; on: redrawn and woken
