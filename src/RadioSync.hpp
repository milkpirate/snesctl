//
// Run a function shortly before every radio event (connection or advertising event) of the SoftDevice, e.g. to read
// inputs right before they can be sent, instead of waiting for the next connection event after an independent timer.
//

#pragma once

#include <cstdint>


namespace RadioSync {
    using Callback = void (*)();

    // `callback` is called from a dedicated (high priority) task, `distance` (NRF_RADIO_NOTIFICATION_DISTANCE_*) before
    // each radio event. To be called once, right after `Bluefruit.begin()` (no radio activity yet).
    void begin(uint8_t distance, Callback callback);
}
