#include <Arduino.h>
#include "buttons.h"
#include "ui.h"

static const int BOOT_BUTTON_PIN = 0;
static const int PWR_BUTTON_PIN = 17;

static const unsigned long DEBOUNCE_MS = 30;
static const unsigned long HOLD_MS = 2000;

struct Button {
  int pin;
  const char *name;
  bool lastState;
  unsigned long lastChange;
  bool isPressed;            // debounced: true from an accepted press until its release
  unsigned long pressedAt;
  bool holdFired;            // the hold action already ran for this press
};

static Button buttons[] = {
  {BOOT_BUTTON_PIN, "BOOT", HIGH, 0, false, 0, false},
  {PWR_BUTTON_PIN,  "PWR",  HIGH, 0, false, 0, false},
};

// The moment a button goes down. BOOT acts here, so screen switching feels instant.
static void on_press(Button &b) {
    if (b.pin == BOOT_BUTTON_PIN) {
        ui_next_screen();
    }
}

// Released before HOLD_MS. PWR acts here rather than on press, because until the
// release it could still turn out to be a hold.
static void on_short_press(Button &b) {
    if (b.pin == PWR_BUTTON_PIN) {
        ui_toggle_screen();
    }
}

// Still held after HOLD_MS. Runs once per press, while the button is still down.
static void on_hold(Button &b) {
    if (b.pin == PWR_BUTTON_PIN) {
        Serial.println("PWR hold: power off comes at roadmap step 8");
    }
}

void buttons_init(void) {
    for (Button &b : buttons) {
        pinMode(b.pin, INPUT_PULLUP);
    }
}

void buttons_poll(void) {
    unsigned long now = millis();

    for (Button &b : buttons) {
        bool reading = digitalRead(b.pin);
        if (reading != b.lastState) {
            // Ignore changes too soon after the previous one: contact bounce
            if ((now - b.lastChange) > DEBOUNCE_MS) {
                if (reading == LOW) {
                    b.isPressed = true;
                    b.pressedAt = now;
                    b.holdFired = false;
                    Serial.printf("%s pressed\n", b.name);
                    on_press(b);
                } else if (b.isPressed) {
                    b.isPressed = false;
                    if (!b.holdFired) {
                        on_short_press(b);
                    }
                }
            }
            b.lastChange = now;
            b.lastState = reading;
        }

        if (b.isPressed && !b.holdFired && (now - b.pressedAt) >= HOLD_MS) {
            b.holdFired = true;
            on_hold(b);
        }
    }
}
