//
// Status LED: flashes briefly while waiting for a connection (advertising), off otherwise.
//

#pragma once

#include <cstdint>


namespace StatusLed {
    // flash for `on_ms` every `period_ms` while blinking
    void begin(uint32_t pin, uint8_t on_level, uint32_t on_ms, uint32_t period_ms);

    void startBlinking();
    void stopBlinking();    // the LED is turned off with the next tick (at most `on_ms` later)

    void off();             // right away, e.g. before entering system off
}
