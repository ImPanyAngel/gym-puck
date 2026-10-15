#pragma once

#include "lvgl.h"

// Called from inside ui.cpp, which already holds the LVGL lock.
lv_obj_t * ui_music_create(void);

// Disconnected: shows "Not connected" and clears the track
void ui_music_set_connected(bool connected);

void ui_music_set_title(const char *title);     // empty shows "Nothing playing"
void ui_music_set_artist(const char *artist);
void ui_music_set_duration(float seconds);

// From the phone's playback info. The screen moves the progress on by itself from here,
// at `rate` seconds per second (0 while paused).
void ui_music_set_playback(bool playing, float rate, float elapsed_s);
