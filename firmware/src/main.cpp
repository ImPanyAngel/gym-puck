#include <Arduino.h>
#include "lvgl_port.h"
#include "buttons.h"
#include "ui.h"
#include "ble.h"
#include "gym_log.h"

void setup() {
    Serial.begin(115200);

    buttons_init();
    gym_log_init();
    Lvgl_PortInit();
    ui_init();
    ble_init();
}

void loop() {
    buttons_poll();
    delay(5);
}
