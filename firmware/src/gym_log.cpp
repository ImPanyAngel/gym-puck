#include <Arduino.h>
#include <Preferences.h>
#include "gym_log.h"

// Preferences wraps the ESP32's NVS: a small key-value store in its own area of flash,
// separate from the program, so uploading new firmware doesn't erase it.
static Preferences prefs;

// One key per month, e.g. "2026-10"
static void month_key(int year, int month, char * key, size_t size) {
    snprintf(key, size, "%04d-%02d", year, month);
}

void gym_log_init(void) {
    prefs.begin("gym", false);   // "gym" groups our keys; false = read and write
}

uint32_t gym_log_get_month(int year, int month) {
    char key[8];
    month_key(year, month, key, sizeof(key));
    return prefs.getUInt(key, 0);   // 0, no days, for a month never saved
}

void gym_log_set_day(int year, int month, int day, bool went) {
    if (day < 1 || day > 31) {
        return;
    }
    char key[8];
    month_key(year, month, key, sizeof(key));

    uint32_t bits = prefs.getUInt(key, 0);
    if (went) {
        bits |= (1u << day);     // set bit `day`
    } else {
        bits &= ~(1u << day);    // clear bit `day`
    }
    prefs.putUInt(key, bits);
    Serial.printf("Gym log: %s day %d %s\n", key, day, went ? "saved" : "cleared");
}
