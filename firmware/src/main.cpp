#include <Arduino.h>
#include "lvgl_port.h"
#include "buttons.h"
#include "ui.h"

void setup() {
    Serial.begin(115200);

    buttons_init();
    Lvgl_PortInit();
    ui_init();
}

void loop() {
    buttons_poll();
    delay(5);
}
