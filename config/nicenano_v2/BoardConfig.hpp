//
// nice!nano v2 specific configuration, selected by the PlatformIO environment `nicenano_v2`.
// Values from https://github.com/zmkfirmware/zmk/blob/main/app/boards/nicekeyboards/nice_nano/nice_nano_nrf52840_zmk_2_0_0.overlay
//

#pragma once

#include <Arduino.h>


namespace Config::Board {
    constexpr auto name = "nice!nano v2";

    // external VCC switch (P0.13), powering the controller, is active high
    constexpr uint8_t ctl_vcc_on = HIGH;

    // the internal DC/DC converter (REG1) needs external inductors, which are not verified for this board,
    // so the default linear regulator is used
    constexpr bool dcdc_enabled = false;

    // battery is connected to VDDH, measured via the ADC's internal VDDH/5 input. A full battery (4.2V) results in
    // 0.84V, within the ADC range of 3.6V (see Config::Battery). No external divider, so the default acquisition
    // time is fine.
    inline uint32_t read_battery_adc() {
        return analogReadVDDHDIV5();
    }
    constexpr auto battery_division_factor = 5.0f;
}
