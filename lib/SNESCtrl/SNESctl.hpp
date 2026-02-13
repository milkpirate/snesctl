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
            hid_gamepad_report_t get_gamepad_report() const override;
            const static SPISettings get_spi_settings();

        private:
            uint16_t read_raw_() const;
            hid_gamepad_report_t get_gamepad_report_from_raw_(const uint16_t raw) const;

            Interfaces::SPI& spi_;
            const Interfaces::GPIO& latch_;
            const Interfaces::GPIO& vcc_;
            const Interfaces::GPIO& gnd_;
    };
}