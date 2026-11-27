#include "StatusLed.hpp"

#include <Arduino.h>


namespace {
    SoftwareTimer timer;
    uint32_t led_pin;
    uint8_t led_on_level;
    uint32_t ticks_per_period;
    volatile bool blinking = false;

    void set_led(const bool on) {
        digitalWrite(led_pin, on ? led_on_level : !led_on_level);
    }

    // runs every `on_ms`: the LED is on for the first tick of each period. Turning it off and stopping the timer is
    // done here too, so it can not race with a tick that would turn it on again.
    void tick(TimerHandle_t) {
        static uint32_t count = 0;

        if (!blinking) {
            set_led(false);
            count = 0;
            timer.stop();
            return;
        }
        set_led(count == 0);
        count = (count + 1) % ticks_per_period;
    }
}

namespace StatusLed {
    void begin(const uint32_t pin, const uint8_t on_level, const uint32_t on_ms, const uint32_t period_ms) {
        led_pin = pin;
        led_on_level = on_level;
        ticks_per_period = period_ms / on_ms;
        pinMode(led_pin, OUTPUT);
        set_led(false);
        timer.begin(on_ms, tick);
    }

    void startBlinking() {
        blinking = true;
        timer.start();
    }

    void stopBlinking() {
        blinking = false;
    }

    void off() {
        blinking = false;
        set_led(false);
    }
}
