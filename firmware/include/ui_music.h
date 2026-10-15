#pragma once

#include "lvgl.h"

// Called from inside ui.cpp, which already holds the LVGL lock.
lv_obj_t * ui_music_create(void);
void ui_music_set_track(const char *title, const char *artist);
void ui_music_set_progress(int elapsed_s, int duration_s);
void ui_music_set_playing(bool playing);
