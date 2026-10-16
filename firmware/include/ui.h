#pragma once

// These take the LVGL lock themselves, so call them from setup(), loop() or a
// Bluetooth callback, never from inside an LVGL event callback.
void ui_init(void);
void ui_next_screen(void);   // does nothing while the screen is off
void ui_toggle_screen(void); // off: panel asleep, touch ignored; on: redrawn and woken

// Now playing, from the iPhone
void ui_set_connected(bool connected);
void ui_set_title(const char *title);
void ui_set_artist(const char *artist);
void ui_set_duration(float seconds);
void ui_set_playback(bool playing, float rate, float elapsed_s);

// Today's date, from the iPhone. Enables "Went today?" and loads saved gym days.
void ui_set_date(int year, int month, int day);
