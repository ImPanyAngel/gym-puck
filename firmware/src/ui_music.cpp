#include "ui_music.h"
#include "ui_colors.h"

static lv_obj_t * screen;
static lv_obj_t * progress_arc;
static lv_obj_t * title_label;
static lv_obj_t * artist_label;
static lv_obj_t * time_label;
static lv_obj_t * play_label;

// A button that shows only its icon: no fill, no shadow, a faint highlight while pressed
static lv_obj_t * create_icon_button(const char *symbol, const lv_font_t *font, int size, int x, int y) {
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_set_size(btn, size, size);
    lv_obj_align(btn, LV_ALIGN_CENTER, x, y);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_text_color(btn, lv_color_hex(UI_COLOR_PRIMARY), 0);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, symbol);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_center(label);

    return btn;
}

lv_obj_t * ui_music_create(void) {
    screen = lv_obj_create(NULL);

    // Black background; text colour set here is inherited by every label on the screen
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BACKGROUND), 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(UI_COLOR_PRIMARY), 0);

    // Progress arc around the edge of the screen, display only
    progress_arc = lv_arc_create(screen);
    lv_obj_set_size(progress_arc, 450, 450);
    lv_obj_center(progress_arc);
    lv_arc_set_rotation(progress_arc, 270);   // 0° is at 3 o'clock, so start at 12 o'clock
    lv_arc_set_bg_angles(progress_arc, 0, 360);
    lv_obj_remove_style(progress_arc, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(progress_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(progress_arc, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_width(progress_arc, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(progress_arc, lv_color_hex(UI_COLOR_TRACK), LV_PART_MAIN);

    // Title scrolls sideways when it's too long for the width
    title_label = lv_label_create(screen);
    lv_obj_set_width(title_label, 300);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_34, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -100);

    artist_label = lv_label_create(screen);
    lv_obj_set_width(artist_label, 280);
    lv_label_set_long_mode(artist_label, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_align(artist_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(artist_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(artist_label, lv_color_hex(UI_COLOR_SECONDARY), 0);
    lv_obj_align(artist_label, LV_ALIGN_CENTER, 0, -58);

    // Playback controls; they do nothing until the iPhone is connected
    create_icon_button(LV_SYMBOL_PREV, &lv_font_montserrat_34, 80, -110, 15);
    lv_obj_t * play_btn = create_icon_button(LV_SYMBOL_PAUSE, &lv_font_montserrat_48, 100, 0, 15);
    play_label = lv_obj_get_child(play_btn, 0);
    create_icon_button(LV_SYMBOL_NEXT, &lv_font_montserrat_34, 80, 110, 15);

    time_label = lv_label_create(screen);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_24, 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 90);

    create_icon_button(LV_SYMBOL_MINUS, &lv_font_montserrat_24, 64, -50, 150);
    create_icon_button(LV_SYMBOL_PLUS, &lv_font_montserrat_24, 64, 50, 150);

    // Placeholder until the iPhone sends real data
    ui_music_set_track("A Very Long Song Title That Scrolls", "Artist Name");
    ui_music_set_progress(60, 225);
    ui_music_set_playing(true);

    return screen;
}

void ui_music_set_track(const char *title, const char *artist) {
    lv_label_set_text(title_label, title);
    lv_label_set_text(artist_label, artist);
}

void ui_music_set_progress(int elapsed_s, int duration_s) {
    lv_arc_set_range(progress_arc, 0, duration_s);
    lv_arc_set_value(progress_arc, elapsed_s);
    lv_label_set_text_fmt(time_label, "%d:%02d / %d:%02d", elapsed_s / 60, elapsed_s % 60, duration_s / 60, duration_s % 60);
}

// The button shows the action a tap would take: pause while playing, play while paused
void ui_music_set_playing(bool playing) {
    lv_label_set_text(play_label, playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
}
