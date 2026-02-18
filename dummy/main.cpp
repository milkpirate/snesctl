#include <Arduino.h>

#define LED 0

void setup() {
    pinMode(LED, OUTPUT);
}

void loop() {
    digitalToggle(LED);
    delay(1000);
}