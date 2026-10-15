#include "ui_gym.h"
#include "ui_colors.h"
#include "ui_calendar.h"

static lv_obj_t * screen;
static lv_obj_t * went_button;

// Runs each time the toggle flips. LVGL callbacks already hold the lock.
static void went_changed(lv_event_t * e) {
    bool went = lv_obj_has_state(went_button, LV_STATE_CHECKED);
    ui_calendar_set_today_marked(went);
}

lv_obj_t * ui_gym_create(void) {
    screen = lv_obj_create(NULL);

    // Black background; text colour set here is inherited by every label on the screen
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BACKGROUND), 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(UI_COLOR_PRIMARY), 0);

    lv_obj_t * question = lv_label_create(screen);
    lv_label_set_text(question, "Went today?");
    lv_obj_set_style_text_font(question, &lv_font_montserrat_34, 0);
    lv_obj_align(question, LV_ALIGN_CENTER, 0, -120);

    // A checkable button stays "checked" after a tap until tapped again, so it
    // remembers yes/no by itself. Unchecked: outline only. Checked: filled.
    went_button = lv_button_create(screen);
    lv_obj_add_flag(went_button, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(went_button, 170, 170);
    lv_obj_align(went_button, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_radius(went_button, 28, 0);
    lv_obj_set_style_shadow_width(went_button, 0, 0);
    lv_obj_set_style_border_width(went_button, 4, 0);
    lv_obj_set_style_border_color(went_button, lv_color_hex(UI_COLOR_PRIMARY), 0);
    lv_obj_set_style_bg_opa(went_button, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_opa(went_button, LV_OPA_COVER, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(went_button, lv_color_hex(UI_COLOR_PRIMARY), LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(went_button, LV_OPA_20, LV_STATE_PRESSED);
    // The tick is a faint hint while unchecked and black on the grey fill once checked
    lv_obj_set_style_text_color(went_button, lv_color_hex(UI_COLOR_TRACK), 0);
    lv_obj_set_style_text_color(went_button, lv_color_hex(UI_COLOR_BACKGROUND), LV_STATE_CHECKED);
    lv_obj_add_event_cb(went_button, went_changed, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * tick = lv_label_create(went_button);
    lv_label_set_text(tick, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(tick, &lv_font_montserrat_48, 0);
    lv_obj_center(tick);

    // Placeholder until the gym log is saved in flash
    ui_gym_set_went_today(false);

    return screen;
}

void ui_gym_set_went_today(bool went) {
    if (went) {
        lv_obj_add_state(went_button, LV_STATE_CHECKED);
    } else {
        lv_obj_remove_state(went_button, LV_STATE_CHECKED);
    }
}
