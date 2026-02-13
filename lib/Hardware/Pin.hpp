//
// Created by Paul Schroeder on 07.02.2026.
//

#pragma once

#include <stdint.h>

#include <Arduino.h>

#include "Interfaces.hpp"


namespace Hardware {
    class Pin : public Interfaces::GPIO {
    public:
        constexpr explicit Pin(uint32_t pin_num) : pin_number_(pin_num) {}

        void pinMode(uint32_t mode) const override { ::pinMode(pin_number_, mode); }
        void digitalWrite(uint32_t value) const override { ::digitalWrite(pin_number_, value); }
        bool digitalRead() const override { return ::digitalRead(pin_number_); }
        void digitalToggle() const override { ::digitalToggle(pin_number_); }
        bool isConnected() const { return true; }

    private:
        const uint32_t pin_number_;
    };

    // No-connection sentinel - minimal implementation
    struct _nc : Interfaces::GPIO{};
    const _nc NC{};
}

