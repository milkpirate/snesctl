//
// Created by Paul Schroeder on 07.02.2026.
//

#include <SPI.h>

#include "SNESctl.hpp"


namespace SNESctl {
    Controller::Controller(
        Interfaces::SPI& spi,
        const Interfaces::GPIO& latch,
        const Interfaces::GPIO& vcc,
        const Interfaces::GPIO& gnd
    ) :
        spi_(spi),
        latch_(latch),
        vcc_(vcc),
        gnd_(gnd)
    {
        if (vcc_.isConnected()) {
            vcc_.pinMode(OUTPUT);
            vcc_.digitalWrite(HIGH);
        }
        if (gnd_.isConnected()) {
            gnd_.pinMode(OUTPUT);
            gnd_.digitalWrite(LOW);
        }

        latch_.pinMode(OUTPUT);
        latch_.digitalWrite(LOW);
    }

    const SPISettings Controller::get_spi_settings() {
        static const SPISettings settings(
                500000,         // 500 kHz, SNES original 100 kHz
                LSBFIRST,
                SPI_MODE2   // glitches? try MODE3
        );
        return settings;
    }

    hid_gamepad_report_t Controller::get_gamepad_report() const {
        return this->get_gamepad_report_from_raw_(this->read_raw_());
    }

    hid_gamepad_report_t Controller::get_gamepad_report_from_raw_(const uint16_t spi_raw) const {
        auto raw = ~spi_raw;     // inverted logic
        hid_gamepad_report_t gp{};

        constexpr uint16_t sSYB_button_mask = 0b0000000000001111;   // Start, Select, Y, B
        constexpr uint16_t RLXA_button_mask = 0b0000111100000000;   // R, L, X, A
        constexpr uint16_t dpad_button_mask = 0b0000000011110000;   // D-pad: Up, Down, Left, Right

        auto buttons_sSYB = (raw & sSYB_button_mask);
        auto buttons_RLXA = (raw & RLXA_button_mask) >> 4;
        auto dpad         = (raw & dpad_button_mask) >> 4;

        gp.buttons = buttons_RLXA | buttons_sSYB;

        auto get_axis = [](bool neg, bool pos) -> int8_t {
            return (pos - neg) * 127;
        };

        constexpr uint8_t up      = bit(0);
        constexpr uint8_t down    = bit(1);
        constexpr uint8_t left    = bit(2);
        constexpr uint8_t right   = bit(3);

        gp.x = get_axis(dpad & left, dpad & right);
        gp.y = get_axis(dpad & up,   dpad & down);    // origin -> upper left
        return gp;
    }

    uint16_t Controller::read_raw_() const {
        constexpr uint8_t reset_time = 10;     // µs
        constexpr uint8_t stabilize_time = 20;  // µs

        latch_.digitalWrite(HIGH);
        delayMicroseconds(stabilize_time);
        latch_.digitalWrite(LOW);
        delayMicroseconds(reset_time);  // ensure CD4021 timing @ 3v3

        this->spi_.beginTransaction(get_spi_settings());
        uint16_t result = this->spi_.transfer16(0x0000);
        this->spi_.endTransaction();
        return result;
    }
}
