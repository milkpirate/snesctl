//
// Created by Paul Schroeder on 07.02.2026.
//

#include <SPI.h>


#include "SNESCtrl.hpp"


namespace SNESCtrl {
    ::SPISettings get_spi_settings() {
        return ::SPISettings(
            4000000,
            MSBFIRST,
            SPI_MODE0
        );
    };

    uint16_t read_raw(SPIClass* const spi, const uint8_t latch) {
        // toogle latch
        digitalWrite(latch, 1);
        digitalWrite(latch, 0);

        return spi->transfer16(0);
    };

    hid_gamepad_report_t get_gamepad_report_from_raw(const uint16_t raw) {
        hid_gamepad_report_t gp;

        //TODO

        return gp;
    };
}
