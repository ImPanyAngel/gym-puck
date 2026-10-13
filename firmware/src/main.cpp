#include <Arduino.h>

void setup() {
    Serial.begin(115200);

    delay(2000); // Wait for Serial to initialize

    ESP.getPsramSize();
    Serial.print("PSRAM Size: ");
    Serial.print(ESP.getPsramSize());
    Serial.println(" bytes");
}

void loop() {
    Serial.println("Hello, World!");
    delay(1000);
}
