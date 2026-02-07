//
// Created by Paul Schroeder on 07.02.2026.
//

#pragma once

#include <bluefruit.h>

namespace Debug {
    void send_test_buttons_presses(BLEHidGamepad &ble_hid_gp, Print &streamer);
    void print_binary(const uint16_t raw_buttons, Print &streamer);
}


