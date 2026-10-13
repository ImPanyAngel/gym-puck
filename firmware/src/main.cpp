#include <Arduino.h>
#include "lvgl.h"
#include "lvgl_port.h"

void user_ui_init(void) {

    lv_obj_t * label1 = lv_label_create(lv_screen_active());
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    lv_label_set_text(label1, "Recolor is not supported for v9 now.");
    lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);

}

void setup() {
    Serial.begin(115200);

    Lvgl_PortInit();
    if (lvgl_lock(0)) {
        user_ui_init();
        lvgl_unlock();
    }

}

void loop() {}
