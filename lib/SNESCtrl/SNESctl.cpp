//
// Created by Paul Schroeder on 07.02.2026.
//

#include <SPI.h>

#include "SNESctl.hpp"
#include "SNESdecode.hpp"


namespace SNESctl {
    Controller::Controller(
        SPIClass& spi,
        const uint32_t latch,
        const uint32_t vcc,
        const uint32_t gnd
    ) :
        spi_(spi),
        latch_(latch),
        vcc_(vcc),
        gnd_(gnd)
    {}

    void Controller::begin() const {
        this->powerOff();
    }

    void Controller::powerOff() const {
        pinMode(this->vcc_, INPUT);
        pinMode(this->gnd_, INPUT);
        pinMode(this->latch_, INPUT);
    }

    void Controller::powerOn() const {
        pinMode(this->vcc_, OUTPUT);
        pinMode(this->gnd_, OUTPUT);
        pinMode(this->latch_, OUTPUT);

        digitalWrite(this->vcc_, HIGH);
        digitalWrite(this->gnd_, LOW);
        digitalWrite(this->latch_, LOW);
    }

    void Controller::printRawButtons(Print& streamer, const uint16_t raw) {
        const auto raw_buttons = ~raw; // invert logic for easier visualization (pressed -> 1, not pressed -> 0)
        // button order in raw data (B, Y, Start, Select, Up, Down, Left, Right, A, X, L, R, 4x unused)
        constexpr char letters[] = "BYSsudlrAXLR####";
        for (int i=0; i<(sizeof letters - 1); ++i) {
            const auto pressed = bitRead(raw_buttons, i);
            const auto symbol = pressed ? letters[i] : '_';
            streamer.print(symbol);
        }
        streamer.printf(" (0x%04hx)\n", raw_buttons);
    }

    void Controller::printReport(Print& streamer, const hid_gamepad_report_t& report) {
        streamer.printf(
            "HID report: x=%4d, y=%4d, z=%4d, rx=%4d, ry=%4d, rz=%4d, hat=%4d, buttons=%s\n",
            report.x, report.y, report.z, report.rx, report.ry, report.rz, report.hat,
            std::bitset<32>(report.buttons).to_string().c_str()
        );
    }

    uint16_t Controller::readRaw() const {
        constexpr uint8_t latch_pulse_us = 12;  // µs - SNES spec requires ~12µs latch pulse
        constexpr uint8_t setup_time_us = 10;   // µs - time after latch falls before first clock

        this->powerOn();
        delayMicroseconds(setup_time_us);   // let all voltage lines settle before latching

        digitalWrite(this->latch_, HIGH);
        delayMicroseconds(latch_pulse_us);
        digitalWrite(this->latch_, LOW);
        delayMicroseconds(setup_time_us);   // ensure CD4021 timing @ 3v3

        this->spi_.beginTransaction(this->spiSettings_);
        auto result = this->spi_.transfer16(0x0000);
        this->spi_.endTransaction();

        this->powerOff();
        return result;
    }

    hid_gamepad_report_t Controller::getGamepadReport() const {
        const auto state = decode(this->readRaw());
        hid_gamepad_report_t gp{};
        gp.buttons = state.buttons;
        gp.x = state.x;
        gp.y = state.y;
        return gp;
    }
}
