#include "ui_calendar.h"
#include "ui_colors.h"

// LVGL keeps pointers to these rather than copying them, so they must outlive the calendar
static const char * DAY_NAMES[7] = {"M", "T", "W", "T", "F", "S", "S"};
static const char * MONTH_NAMES[12] = {"January", "February", "March", "April", "May", "June",
                                       "July", "August", "September", "October", "November", "December"};
static lv_calendar_date_t highlighted[31];

static const int CALENDAR_SIZE = 310;               // square, in pixels
static const int ROW_HEIGHT = CALENDAR_SIZE / 7;    // day names + 6 weeks
static const int CALENDAR_Y_ALL_ROWS = 12;          // position when a month uses all 6 rows
static const int LABEL_ABOVE_CALENDAR = -194;       // month name, relative to the calendar

static lv_obj_t * screen;
static lv_obj_t * month_label;
static lv_obj_t * calendar;

static int today_year;
static int today_month;
static int today_day;
static bool marked[32];   // marked[1] to marked[31]; index 0 unused so day numbers match

// Hands LVGL the list of marked days in the shown month
static void update_highlights(void) {
    int count = 0;
    for (int day = 1; day <= 31; day++) {
        if (marked[day]) {
            highlighted[count].year = today_year;
            highlighted[count].month = today_month;
            highlighted[count].day = day;
            count++;
        }
    }
    lv_calendar_set_highlighted_dates(calendar, highlighted, count);
}

// The calendar flags marked days with this; LVGL only names it inside lv_calendar.c
static const lv_buttonmatrix_ctrl_t CTRL_MARKED = LV_BUTTONMATRIX_CTRL_CUSTOM_2;

// Runs just before each part of a day cell (its circle or its number) is drawn, after the
// calendar's own adjustments, so these changes win:
// - Marked days: a solid grey circle with a black number, instead of a faint fill.
// - Days from the previous or next month, which the calendar flags as disabled: hidden.
//   The day names (the first 7 cells) are flagged disabled too, so they are skipped.
static void style_day_cells(lv_event_t * e) {
    lv_obj_t * days = lv_event_get_current_target_obj(e);
    lv_draw_task_t * task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t * base = (lv_draw_dsc_base_t *) lv_draw_task_get_draw_dsc(task);
    if (base->part != LV_PART_ITEMS) {
        return;
    }

    uint32_t cell = base->id1;
    lv_draw_fill_dsc_t * fill = lv_draw_task_get_fill_dsc(task);
    lv_draw_label_dsc_t * label = lv_draw_task_get_label_dsc(task);

    if (lv_buttonmatrix_has_button_ctrl(days, cell, CTRL_MARKED)) {
        if (fill) {
            fill->opa = LV_OPA_COVER;
            fill->color = lv_color_hex(UI_COLOR_PRIMARY);
        }
        if (label) {
            label->color = lv_color_hex(UI_COLOR_BACKGROUND);
        }
    }
    else if (cell >= 7 && lv_buttonmatrix_has_button_ctrl(days, cell, LV_BUTTONMATRIX_CTRL_DISABLED)) {
        if (label) {
            label->opa = LV_OPA_TRANSP;
        }
    }
}

static bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

static int days_in_month(int year, int month) {
    static const int DAYS[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return DAYS[month - 1];
}

// 0 = Monday ... 6 = Sunday. Sakamoto's method, a standard short formula for the weekday.
static int weekday(int year, int month, int day) {
    static const int OFFSETS[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (month < 3) {
        year -= 1;
    }
    int sunday_first = (year + year / 4 - year / 100 + year / 400 + OFFSETS[month - 1] + day) % 7;
    return (sunday_first + 6) % 7;
}

// The calendar always reserves 6 rows of weeks, but a month uses 4 to 6 of them, from the
// top. Moving it down by half a row per unused row keeps the visible days centred.
static void center_on_month(int year, int month) {
    int cells_used = weekday(year, month, 1) + days_in_month(year, month);
    int rows_used = (cells_used + 6) / 7;   // divide by 7, rounding up
    int unused_rows = 6 - rows_used;

    int calendar_y = CALENDAR_Y_ALL_ROWS + unused_rows * ROW_HEIGHT / 2;
    lv_obj_align(calendar, LV_ALIGN_CENTER, 0, calendar_y);
    lv_obj_align(month_label, LV_ALIGN_CENTER, 0, calendar_y + LABEL_ABOVE_CALENDAR);
}

lv_obj_t * ui_calendar_create(void) {
    screen = lv_obj_create(NULL);

    // Black background; text colour set here is inherited by every label on the screen
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_BACKGROUND), 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(UI_COLOR_PRIMARY), 0);

    // Both are positioned in center_on_month(), as their height depends on the month
    month_label = lv_label_create(screen);
    lv_obj_set_style_text_font(month_label, &lv_font_montserrat_34, 0);

    // Small enough that its corners stay inside the round screen
    calendar = lv_calendar_create(screen);
    lv_obj_set_size(calendar, CALENDAR_SIZE, CALENDAR_SIZE);
    lv_calendar_set_day_names(calendar, DAY_NAMES);
    lv_obj_remove_flag(calendar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(calendar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(calendar, 0, 0);
    lv_obj_set_style_text_font(calendar, &lv_font_montserrat_24, 0);

    // The grid of days is a button matrix inside the calendar; display only
    lv_obj_t * days = lv_calendar_get_btnmatrix(calendar);
    lv_obj_remove_flag(days, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(days, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(days, 0, 0);
    // Each day is a circle, black on black so it can't be seen. It has to be opaque rather
    // than transparent: LVGL skips drawing anything transparent, and then there would be
    // no fill or border for style_day_cells and the calendar to recolour for marked days
    // and today.
    lv_obj_set_style_radius(days, LV_RADIUS_CIRCLE, LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(days, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(days, lv_color_hex(UI_COLOR_BACKGROUND), LV_PART_ITEMS);
    lv_obj_set_style_border_width(days, 2, LV_PART_ITEMS);
    lv_obj_set_style_border_opa(days, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_border_color(days, lv_color_hex(UI_COLOR_BACKGROUND), LV_PART_ITEMS);
    lv_obj_add_event_cb(days, style_day_cells, LV_EVENT_DRAW_TASK_ADDED, NULL);

    // Placeholder until the iPhone sends the date
    ui_calendar_set_today(2026, 10, 16);

    // Example gym days to preview the look; remove once days are saved in flash
    const int example_days[] = {1, 3, 6, 8, 10, 13, 15};
    for (int day : example_days) {
        ui_calendar_set_day_marked(day, true);
    }

    return screen;
}

void ui_calendar_set_today(int year, int month, int day) {
    // A new month starts with nothing marked
    if (year != today_year || month != today_month) {
        for (int i = 0; i < 32; i++) {
            marked[i] = false;
        }
    }

    today_year = year;
    today_month = month;
    today_day = day;

    lv_calendar_set_today_date(calendar, year, month, day);
    lv_calendar_set_month_shown(calendar, year, month);
    lv_label_set_text(month_label, MONTH_NAMES[month - 1]);
    center_on_month(year, month);
    update_highlights();
}

void ui_calendar_set_day_marked(int day, bool is_marked) {
    if (day < 1 || day > 31) {
        return;
    }
    marked[day] = is_marked;
    update_highlights();
}

void ui_calendar_set_today_marked(bool is_marked) {
    ui_calendar_set_day_marked(today_day, is_marked);
}
