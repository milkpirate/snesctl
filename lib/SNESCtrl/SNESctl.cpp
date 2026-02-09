//
// Created by Paul Schroeder on 07.02.2026.
//

#include <SPI.h>


#include "SNESctl.hpp"


namespace SNESctl {
    ::SPISettings get_spi_settings() {
        return ::SPISettings(
            500000,         // 500 kHz, SNES original 100 kHz
            LSBFIRST,
            SPI_MODE2   // glictches? try MODE3
        );
    };

    uint16_t read_raw(SPIClass* const spi, const uint8_t latch) {
        constexpr auto reset_time = 10;
        constexpr auto stablize_time = 20;

        digitalWrite(latch, LOW);
        delayMicroseconds(reset_time);  // ensure CD4021 timimngs @ 3v3
        digitalWrite(latch, HIGH);
        delayMicroseconds(stablize_time);

        return spi->transfer16(0);
    };

    hid_gamepad_report_t get_gamepad_report_from_raw(uint16_t raw) {
        raw = ~raw;     // inverted logic
        hid_gamepad_report_t gp;

        auto buttons_sSYB = (raw & 0x000f);
        auto buttons_RLXA = (raw & 0x0f00) >> 4;
        gp.buttons = buttons_RLXA | buttons_sSYB;

        constexpr auto up      = bit(0);
        constexpr auto down    = bit(1);
        constexpr auto left    = bit(2);
        constexpr auto right   = bit(3);

        auto dpad         = (raw & 0x00f0) >> 4;

        auto get_axis = [](bool neg, bool pos) -> int8_t {
            return (pos - neg) * 127;
        };

        gp.x = get_axis(dpad & left, dpad & right);
        gp.y = get_axis(dpad & up,   dpad & down);    // origine -> upper left
        return gp;
    };
}
