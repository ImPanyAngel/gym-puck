#include <Arduino.h>
#include "lvgl.h"
#include "lvgl_port.h"

// Function prototypes
void user_interface(void);

void setup() {
    Serial.begin(115200);

    Lvgl_PortInit();
    if (lvgl_lock(0)) {
        user_interface();
        lvgl_unlock();
    }
}

void loop() {}

// variable declarations
int press_count = 0;
lv_obj_t * label;

static void event_handler(lv_event_t * e) {
    lv_label_set_text_fmt(label, "button pressed: %d", ++press_count);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60);
}

void user_interface(void) {

    label = lv_label_create(lv_screen_active());
    lv_obj_t * btn1 =  lv_button_create(lv_screen_active());
    lv_obj_t * btn1_label = lv_label_create(btn1);

    lv_label_set_text(label, "button pressed: 0");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60);

    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

    lv_label_set_text(btn1_label, "Button");
    lv_obj_center(btn1_label);

}
