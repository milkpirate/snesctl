//
// SuperMini nRF52840 (aka "ProMicro NRF52840") specific configuration, selected by the PlatformIO environment
// `supermini`. A nice!nano v2 clone, see https://github.com/joric/nrfmicro/wiki/Alternatives#supermini-nrf52840
// and the reverse engineered design https://github.com/sasodoma/nrf52840-promicro
//

#pragma once

#include <Arduino.h>


namespace Config::Board {
    constexpr auto name = "SuperMini nRF52840";

    // external VCC (P0.13 -> enable of the external 3.3V regulator), powering the controller, is active high.
    // NOTE: early batches have a 5.6k pull-up on this line, leaking ~700uA while VCC is switched off.
    // Check with `task vcc-test`, see tools/vcc-test/main.cpp.
    constexpr uint8_t ctl_vcc_on = HIGH;

    // the internal DC/DC converter (REG1) has its inductors fitted (10uH + 15nH), so it can be used. It supplies
    // the CPU and radio more efficiently than the default linear regulator.
    constexpr bool dcdc_enabled = true;

    // battery is connected to VDDH, measured via the ADC's internal VDDH/5 input (the board's battery voltage
    // divider is connected to P0.24, not to an analog input). While USB is plugged in, VDDH is supplied by USB.
    inline uint32_t read_battery_adc() {
        return analogReadVDDHDIV5();
    }
    constexpr auto battery_division_factor = 5.0f;
}
