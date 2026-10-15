#include <Arduino.h>
#include "buttons.h"
#include "ui.h"

struct Button {
  int pin;
  const char *name;
  bool lastState;
  unsigned long lastChange;
};

static Button buttons[] = {
  {0,  "Button GPIO 0",  HIGH, 0},
  {17, "Button GPIO 17", HIGH, 0},
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
                ui_show_button_press(b.name);
            }
            b.lastChange = millis();
            b.lastState = reading;
        }
    }
}
