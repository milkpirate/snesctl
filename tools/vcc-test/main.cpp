//
// Measure the leak of the external VCC switch's pull-up (SuperMini nRF52840, nice!nano v2): `task vcc-test`
//
// Early SuperMini batches have a 5.6k pull-up on the enable line of the external 3.3V regulator (P0.13). While VCC
// is switched off (P0.13 low), ~700uA flow through it. Later batches use 10M or ~500k (< 10uA).
//
// 1. flash this firmware, unplug USB (VDDH is supplied by USB otherwise), power cycle via the battery
// 2. put a multimeter (mA/uA range) in series with the battery
// 3. VCC toggles every 20s, announced by the LED: 1 flash = VCC on, 2 flashes = VCC off
// 4. compare the current of both phases: a difference of ~0.7mA means the 5.6k pull-up
//
// Bluetooth is not started and the controller pins are left floating, so only the VCC switch changes.
//

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>  // USB serial

namespace {
    constexpr auto vcc_pin = PIN_013;           // HIGH = on (SuperMini, nice!nano v2)
    constexpr auto led_pin = LED_BUILTIN;
    constexpr uint32_t phase_ms = 20 * 1000;

    constexpr uint32_t controller_pins[] = {PIN_031, PIN_002, PIN_029, PIN_115, PIN_113};

    void flash(const uint8_t times) {
        for (uint8_t i = 0; i < times; ++i) {
            digitalWrite(led_pin, LED_STATE_ON);
            delay(100);
            digitalWrite(led_pin, !LED_STATE_ON);
            delay(200);
        }
    }

    void vcc_phase(const bool on) {
        digitalWrite(vcc_pin, on ? HIGH : LOW);
        Serial.printf("VCC %s for %lus\n", on ? "on" : "off", phase_ms / 1000);
        flash(on ? 1 : 2);
        delay(phase_ms);
    }
}

void setup() {
    Serial.begin(115200);
    for (const auto pin : controller_pins) pinMode(pin, INPUT);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, !LED_STATE_ON);
    pinMode(vcc_pin, OUTPUT);
}

void loop() {
    vcc_phase(true);
    vcc_phase(false);
}
