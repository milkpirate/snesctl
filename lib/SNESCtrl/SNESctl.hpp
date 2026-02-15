//
// Created by Paul Schroeder on 07.02.2026.
//

#pragma once

#include <bluefruit.h>

#include "Interfaces.hpp"
#include "Pin.hpp"


namespace SNESctl {

class Controller : public Interfaces::GamepadReporter {
    public:
        Controller(
            Interfaces::SPI& spi,
            const Interfaces::GPIO& latch,
            const Interfaces::GPIO& vcc = Hardware::NC,
            const Interfaces::GPIO& gnd = Hardware::NC
        );
        hid_gamepad_report_t getGamepadReport() const override;
        void printButtons(Print& streamer) const override;
        const static SPISettings getSPISettings();

    private:
        hid_gamepad_report_t getGamepadReportFromRaw_(const uint16_t raw) const;
        uint16_t readRaw_() const;

        Interfaces::SPI& spi_;
        const Interfaces::GPIO& latch_;
        const Interfaces::GPIO& vcc_;
        const Interfaces::GPIO& gnd_;
    };
}