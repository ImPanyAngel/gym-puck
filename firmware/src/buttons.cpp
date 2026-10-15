#include <Arduino.h>
#include "buttons.h"
#include "ui.h"

static const int BOOT_BUTTON_PIN = 0;
static const int PWR_BUTTON_PIN = 17;

struct Button {
  int pin;
  const char *name;
  bool lastState;
  unsigned long lastChange;
};

static Button buttons[] = {
  {BOOT_BUTTON_PIN, "BOOT", HIGH, 0},
  {PWR_BUTTON_PIN,  "PWR",  HIGH, 0},
};

void buttons_init(void) {
    for (Button &b : buttons) {
        pinMode(b.pin, INPUT_PULLUP);
    }
}

void buttons_poll(void) {
    for (Button &b : buttons) {
        bool reading = digitalRead(b.pin);
        if (reading != b.lastState) {
            if (reading == LOW && (millis() - b.lastChange) > 30) {
                Serial.println(b.name);
                if (b.pin == BOOT_BUTTON_PIN) {
                    ui_next_screen();
                }
            }
            b.lastChange = millis();
            b.lastState = reading;
        }
    }
}
