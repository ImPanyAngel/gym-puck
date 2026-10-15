#include "ui.h"
#include "lvgl.h"
#include "lvgl_port.h"
#include "ui_colors.h"
#include "ui_music.h"
#include "ui_gym.h"
#include "ui_calendar.h"

// In the order BOOT cycles through them
static const int SCREEN_COUNT = 3;
static lv_obj_t * screens[SCREEN_COUNT];
static int current_screen = 0;

void ui_init(void) {
    if (!lvgl_lock(0)) { return; }

    // Must come before any widgets are created, as the theme is applied at creation
    lv_display_t * display = lv_display_get_default();
    lv_theme_t * theme = lv_theme_default_init(display, lv_color_hex(UI_COLOR_PRIMARY), lv_color_hex(UI_COLOR_TRACK), true, LV_FONT_DEFAULT);
    lv_display_set_theme(display, theme);

    screens[0] = ui_music_create();
    screens[1] = ui_gym_create();
    screens[2] = ui_calendar_create();
    lv_screen_load(screens[current_screen]);

    lvgl_unlock();
}

void ui_next_screen(void) {
    if (!lvgl_lock(0)) { return; }

    // % wraps the count back to 0 after the last screen
    current_screen = (current_screen + 1) % SCREEN_COUNT;
    // false: keep the old screen, so it can be shown again later
    lv_screen_load_anim(screens[current_screen], LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);

    lvgl_unlock();
}
