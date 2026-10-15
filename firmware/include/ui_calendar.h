#pragma once

#include "lvgl.h"

// Called from inside ui.cpp or an LVGL event callback, which already hold the LVGL lock.
lv_obj_t * ui_calendar_create(void);

// Shows the month containing this date, with today outlined. month is 1-12.
void ui_calendar_set_today(int year, int month, int day);

// Fills in (or clears) a day of the shown month as a gym day
void ui_calendar_set_day_marked(int day, bool is_marked);
void ui_calendar_set_today_marked(bool is_marked);
