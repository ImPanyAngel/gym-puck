#pragma once

#include <stdint.h>

// Gym days, saved in flash so they survive restarts and power-off.
// Each month is one 32-bit number where bit N is day N (bit 0 unused): 4 bytes a month.

void gym_log_init(void);   // once, at start-up

uint32_t gym_log_get_month(int year, int month);
void gym_log_set_day(int year, int month, int day, bool went);

inline bool gym_log_has_day(uint32_t month_bits, int day) {
    return (month_bits >> day) & 1;
}
