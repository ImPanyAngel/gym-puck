#include <Arduino.h>
#include "lvgl.h"
#include "lvgl_port.h"

// Prototypes
void user_interface(void);

struct Button {
  int pin;
  const char *name;
  bool lastState;
  unsigned long lastChange;
};

Button buttons[] = {
  {0,  "Button GPIO 0",  HIGH, 0},
  {17, "Button GPIO 17", HIGH, 0},
};

// Declarations
int press_count = 0;
lv_obj_t * label;
lv_obj_t * label_physical;

void setup() {
    Serial.begin(115200);

    for (Button &b : buttons) {
      pinMode(b.pin, INPUT_PULLUP);
  }

    Lvgl_PortInit();
    if (lvgl_lock(0)) {
        user_interface();
        lvgl_unlock();
    }
}

void loop() {
    for (Button &b : buttons) {
        bool reading = digitalRead(b.pin);
        if (reading != b.lastState) {
            if (reading == LOW && (millis() - b.lastChange) > 30) {
                if (lvgl_lock(0)) {
                    lv_label_set_text_fmt(label_physical, "button pressed: %s", b.name);
                    Serial.println(b.name);
                    lvgl_unlock();
                }
            }
            b.lastChange = millis();
            b.lastState = reading;
        }
    }
    delay(5);
}

static void event_handler(lv_event_t * e) { lv_label_set_text_fmt(label, "button pressed: %d", ++press_count); }

void user_interface(void) {

    label = lv_label_create(lv_screen_active());
    label_physical = lv_label_create(lv_screen_active());
    lv_obj_t * btn1 =  lv_button_create(lv_screen_active());
    lv_obj_t * btn1_label = lv_label_create(btn1);

    lv_label_set_text(label, "button pressed: 0");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60);

    lv_label_set_text(label_physical, "button pressed: null");
    lv_obj_align(label_physical, LV_ALIGN_CENTER, 0, 100);

    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    lv_label_set_text(btn1_label, "Button");
    lv_obj_center(btn1_label);

}
