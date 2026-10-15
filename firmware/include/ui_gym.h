#pragma once

#include "lvgl.h"

// Called from inside ui.cpp, which already holds the LVGL lock.
lv_obj_t * ui_gym_create(void);
void ui_gym_set_went_today(bool went);
