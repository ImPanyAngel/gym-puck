#include "ui_music.h"
#include "ui_colors.h"
#include "ams.h"

static lv_obj_t * screen;
static lv_obj_t * progress_arc;
static lv_obj_t * title_label;
static lv_obj_t * artist_label;
static lv_obj_t * time_label;
static lv_obj_t * play_label;

// Playback position, as last reported by the phone. The phone only sends updates when
// something changes, so the current position is worked out from these and the clock.
static float duration_s = 0;
static float elapsed_at_update_s = 0;
static float playback_rate = 0;           // 1.0 while playing, 0.0 while paused
static uint32_t update_time_ms = 0;       // when the last playback update arrived
static int shown_second = -1;             // what the screen shows now, to skip needless redraws

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

// Where playback is now: the last reported position, moved on by the time since then
static void update_progress(lv_timer_t * timer) {
    float since_update_s = lv_tick_elaps(update_time_ms) / 1000.0f;
    float elapsed_s = elapsed_at_update_s + playback_rate * since_update_s;
    if (elapsed_s > duration_s) {
        elapsed_s = duration_s;
    }
    if (elapsed_s < 0) {
        elapsed_s = 0;
    }

    int elapsed = (int) elapsed_s;
    int duration = (int) duration_s;
    if (elapsed == shown_second) {
        return;   // same second as last time: nothing visible would change
    }
    shown_second = elapsed;

    // An arc can't have a range of 0 to 0, so give it at least 1
    lv_arc_set_range(progress_arc, 0, duration > 0 ? duration : 1);
    lv_arc_set_value(progress_arc, elapsed);

    if (duration > 0) {
        lv_label_set_text_fmt(time_label, "%d:%02d / %d:%02d", elapsed / 60, elapsed % 60, duration / 60, duration % 60);
    } else {
        lv_label_set_text(time_label, "");
    }
}

// Runs inside LVGL's task when a control is tapped. ams_send_command only queues the
// command, so the screen never waits for the phone.
static void on_control_clicked(lv_event_t * e) {
    AmsCommand command = (AmsCommand)(uintptr_t) lv_event_get_user_data(e);
    ams_send_command(command);
}

static void add_command(lv_obj_t * btn, AmsCommand command) {
    lv_obj_add_event_cb(btn, on_control_clicked, LV_EVENT_CLICKED, (void *)(uintptr_t) command);
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

    // Playback controls. Each button carries its AMS command as user data.
    lv_obj_t * prev_btn = create_icon_button(LV_SYMBOL_PREV, &lv_font_montserrat_34, 80, -110, 15);
    lv_obj_t * play_btn = create_icon_button(LV_SYMBOL_PAUSE, &lv_font_montserrat_48, 100, 0, 15);
    play_label = lv_obj_get_child(play_btn, 0);
    lv_obj_t * next_btn = create_icon_button(LV_SYMBOL_NEXT, &lv_font_montserrat_34, 80, 110, 15);
    add_command(prev_btn, AMS_COMMAND_PREVIOUS_TRACK);
    add_command(play_btn, AMS_COMMAND_TOGGLE_PLAY_PAUSE);
    add_command(next_btn, AMS_COMMAND_NEXT_TRACK);

    time_label = lv_label_create(screen);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_24, 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 90);

    lv_obj_t * volume_down_btn = create_icon_button(LV_SYMBOL_MINUS, &lv_font_montserrat_24, 64, -50, 150);
    lv_obj_t * volume_up_btn = create_icon_button(LV_SYMBOL_PLUS, &lv_font_montserrat_24, 64, 50, 150);
    add_command(volume_down_btn, AMS_COMMAND_VOLUME_DOWN);
    add_command(volume_up_btn, AMS_COMMAND_VOLUME_UP);

    // Recalculate the position twice a second. LVGL timers run in LVGL's own task,
    // which already holds the lock.
    lv_timer_create(update_progress, 500, NULL);

    ui_music_set_connected(false);

    return screen;
}

void ui_music_set_connected(bool connected) {
    if (connected) {
        // Real values follow straight away from the phone
        return;
    }
    lv_label_set_text(title_label, "Not connected");
    lv_label_set_text(artist_label, "");
    duration_s = 0;
    ui_music_set_playback(false, 0, 0);
}

void ui_music_set_title(const char *title) {
    lv_label_set_text(title_label, title[0] != '\0' ? title : "Nothing playing");
}

void ui_music_set_artist(const char *artist) {
    lv_label_set_text(artist_label, artist);
}

void ui_music_set_duration(float seconds) {
    duration_s = seconds;
    shown_second = -1;   // force the time label to redraw with the new total
    update_progress(NULL);
}

void ui_music_set_playback(bool playing, float rate, float elapsed_s) {
    elapsed_at_update_s = elapsed_s;
    playback_rate = rate;
    update_time_ms = lv_tick_get();

    // The button shows the action a tap would take: pause while playing, play while paused
    lv_label_set_text(play_label, playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);

    shown_second = -1;
    update_progress(NULL);
}
