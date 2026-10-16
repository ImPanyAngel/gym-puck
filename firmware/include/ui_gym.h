#pragma once

#include "lvgl.h"

// Called from inside ui.cpp, which already holds the LVGL lock.
lv_obj_t * ui_gym_create(void);

// The date is now known: enables the toggle, ticked if today is already saved.
// Until this is called, the toggle is disabled.
void ui_gym_set_today(int year, int month, int day, bool went);
