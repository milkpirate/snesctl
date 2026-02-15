//
// Created by Paul Schroeder on 12.02.2026.
//

#pragma once

#include <bluefruit.h>


namespace Interfaces {
    class GamepadReporter {
    public:
        virtual hid_gamepad_report_t getGamepadReport() const = 0;
        virtual void printButtons(Print& streamer) const = 0;
    };

    /**
     * @brief SPI Interface with default Arduino SPI implementation
     *
     * This interface provides default implementations using the standard Arduino SPI library.
     * Subclasses can override these methods to provide custom SPI implementations.
     */
    class SPI {
    public:
        virtual void beginTransaction(const SPISettings settings) const { ::SPI.beginTransaction(settings); }
        virtual void endTransaction() const { ::SPI.endTransaction(); }
        virtual uint16_t transfer16(const uint16_t out) const { return ::SPI.transfer16(out); }
    };

    class GPIO  {
    public:
        // read and write operators for convenient syntax (e.g. latch_pin = HIGH; if (latch_pin) { ... })
        virtual const GPIO& operator=(uint32_t value) const {
            if (this->isConnected()) this->digitalWrite(value);
            return *this;
        }
        virtual operator bool() const {
            if (this->isConnected()) return this->digitalRead();
            return false;
        }

        // Default implementation: compare by pointer address (sadly not by pin number)
        virtual bool operator==(const Interfaces::GPIO& other) const {
            if (other.isConnected()) return false;
            return this == &other;
        }

        // Default no-op implementations
        virtual void pinMode(uint32_t mode) const {}
        virtual void digitalWrite(uint32_t value) const {}
        virtual bool digitalRead() const { return false; }
        virtual void digitalToggle() const {}
        virtual bool isConnected() const { return false; }
        virtual uint32_t getPin() const { return 0; }
    };
}
