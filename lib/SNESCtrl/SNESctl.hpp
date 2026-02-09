//
// Created by Paul Schroeder on 07.02.2026.
//

#pragma once

#include <bluefruit.h>

namespace SNESctl {
    ::SPISettings get_spi_settings();
    uint16_t read_raw(SPIClass* const spi, const uint8_t latch);
    hid_gamepad_report_t get_gamepad_report_from_raw(const uint16_t raw);
}