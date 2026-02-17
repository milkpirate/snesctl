//
// Created by Paul Schroeder on 07.02.2026.
//

#pragma once

#include <bluefruit.h>


namespace Debug {
    void send_test_buttons_presses(BLEHidGamepad &ble_hid_gp, uint32_t delay, Print &streamer);
    void print_gatt_event(uint16_t id, Print &streamer);
}


