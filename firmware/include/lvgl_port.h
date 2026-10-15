#ifndef LVGL_PORT_BSP_H
#define LVGL_PORT_BSP_H

void Lvgl_PortInit(void);

void Lcd_SetBacklight(uint8_t brig);
void Lcd_SetSleep(bool sleep);
void Lcd_SetDisplayOn(bool on);

bool lvgl_lock(int timeout_ms);
void lvgl_unlock(void);


#endif