//
// Created by Paul Schroeder on 13.02.2026.
//

#pragma once

#include <SPI.h>
#include <Arduino.h>

#include "Interfaces.hpp"


namespace Hardware {
    /**
     * @brief Adapter to wrap SPIClass and make it compatible with Interfaces::SPI
     *
     * This allows using the Arduino global ::SPI object or any other SPIClass instance
     * with code that expects an Interfaces::SPI reference.
     */
    class SPIAdapter : public Interfaces::SPI {
    public:
        constexpr SPIAdapter(SPIClass& spi_instance) : spi_(spi_instance) {}
        SPIAdapter(
            const Interfaces::GPIO& miso,
            const Interfaces::GPIO& sck,
            const Interfaces::GPIO& mosi
        ) :
            spi_(::SPI)
        {
            spi_.setPins(miso.getPin(), sck.getPin(), mosi.getPin());
        }

        SPIAdapter(
            const Interfaces::GPIO& miso,
            const Interfaces::GPIO& sck,
            const Interfaces::GPIO& mosi,
            NRF_SPIM_Type *spiBase
        ) :
            spi_(::SPI)
        {
            SPIClass(spiBase, miso.getPin(), sck.getPin(), mosi.getPin());
        }

        // implement interface methods
        void beginTransaction(const SPISettings settings) const override { spi_.beginTransaction(settings); }
        void endTransaction() const override { spi_.endTransaction(); }
        uint16_t transfer16(uint16_t data) const override { return spi_.transfer16(data); }
    private:
        SPIClass& spi_;
    };

    // Global adapter instance wrapping the Arduino ::SPI object
    static SPIAdapter SPI(::SPI);
}