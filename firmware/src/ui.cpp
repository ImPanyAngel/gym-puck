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
static bool screen_on = true;

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

    // BOOT does nothing while the screen is off
    if (!screen_on) {
        lvgl_unlock();
        return;
    }

    // % wraps the count back to 0 after the last screen
    current_screen = (current_screen + 1) % SCREEN_COUNT;
    // false: keep the old screen, so it can be shown again later
    lv_screen_load_anim(screens[current_screen], LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);

    lvgl_unlock();
}

void ui_toggle_screen(void) {
    if (!lvgl_lock(0)) { return; }

    lv_display_t * display = lv_display_get_default();
    lv_indev_t * touch = lv_indev_get_next(NULL);   // the only input device: the touch panel

    if (screen_on) {
        // Ignore touch first, so a tap in a pocket can't do anything
        lv_indev_enable(touch, false);
        // Stop LVGL redrawing: nothing would be seen, and it would only waste power
        lv_display_enable_invalidation(display, false);
        Lcd_SetDisplayOn(false);
        Lcd_SetSleep(true);
    } else {
        Lcd_SetSleep(false);
        // Redraw everything before the panel shows anything, so no stale frame flashes up
        lv_display_enable_invalidation(display, true);
        lv_obj_invalidate(lv_screen_active());
        lv_refr_now(display);
        Lcd_SetDisplayOn(true);
        lv_indev_enable(touch, true);
    }
    screen_on = !screen_on;

    lvgl_unlock();
}

// Each of these only takes the lock and passes the call on to the music screen
void ui_set_connected(bool connected) {
    if (!lvgl_lock(0)) { return; }
    ui_music_set_connected(connected);
    lvgl_unlock();
}

void ui_set_title(const char *title) {
    if (!lvgl_lock(0)) { return; }
    ui_music_set_title(title);
    lvgl_unlock();
}

void ui_set_artist(const char *artist) {
    if (!lvgl_lock(0)) { return; }
    ui_music_set_artist(artist);
    lvgl_unlock();
}

void ui_set_duration(float seconds) {
    if (!lvgl_lock(0)) { return; }
    ui_music_set_duration(seconds);
    lvgl_unlock();
}

void ui_set_playback(bool playing, float rate, float elapsed_s) {
    if (!lvgl_lock(0)) { return; }
    ui_music_set_playback(playing, rate, elapsed_s);
    lvgl_unlock();
}
