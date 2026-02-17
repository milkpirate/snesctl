//
// Created by Paul Schroeder on 07.02.2026.
//

#pragma once

#include <bitset>
#include <bluefruit.h>


namespace SNESctl {

class Controller {
    public:
        static void printRawButtons(Print& streamer, const uint16_t raw);
        static void printReport(Print& streamer, const hid_gamepad_report_t& report);

        Controller(SPIClass& spi, const uint32_t latch, const uint32_t vcc, const uint32_t gnd);

        void begin() const;
        void powerOff() const;
        void powerOn() const;

        hid_gamepad_report_t getGamepadReport() const;
        uint16_t readRaw() const;

    private:
        SPIClass& spi_;

        const uint32_t latch_;
        const uint32_t vcc_;
        const uint32_t gnd_;

        const SPISettings spiSettings_{
            100000,     // SNES controller clock (100 kHz max)
            LSBFIRST,   // SNES sends LSB first
            SPI_MODE0   // CPOL=0 (clock is idle low), CPHA=0 (data is read on rising edge)
        };
    };
}