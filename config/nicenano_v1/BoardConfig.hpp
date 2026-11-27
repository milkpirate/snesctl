//
// nice!nano v1 specific configuration, selected by the PlatformIO environment `nicenano_v1`.
// Values from https://github.com/zmkfirmware/zmk/blob/main/app/boards/nicekeyboards/nice_nano/nice_nano_nrf52840_zmk_1_0_0.overlay
// and https://github.com/zmkfirmware/zmk/blob/main/app/module/drivers/sensor/battery/battery_voltage_divider.c
//

#pragma once

#include <Arduino.h>


namespace Config::Board {
    constexpr auto name = "nice!nano v1";

    // external VCC switch (P0.13), powering the controller, is active low
    constexpr uint8_t ctl_vcc_on = LOW;

    // the internal DC/DC converter (REG1) needs external inductors, which are not verified for this board,
    // so the default linear regulator is used
    constexpr bool dcdc_enabled = false;

    // The battery voltage is measured via a voltage divider on P0.04/AIN2:
    //
    //   BAT+ ---[ R_top 806k ]---+---[ R_bottom 2M ]--- GND
    //                            |
    //                         P0.04 (ADC)
    //
    // V_adc = V_bat * R_bottom / (R_top + R_bottom), so V_bat = V_adc * 1.403.
    // A full battery (4.2V) results in ~3.0V, within the ADC range of 3.6V (see Config::Battery).
    // The large resistors keep the constant current through the divider low (~1.5uA @ 4.2V).
    namespace BatteryDivider {
        constexpr auto r_top_ohm = 806000.0f;
        constexpr auto r_bottom_ohm = 2000000.0f;
        // the ADC sees the divider as source resistance of R_top || R_bottom (~575k), the default acquisition time
        // (3us) is too short to charge the ADC's sampling capacitor through it, so the voltage would read too low.
        // 40us is the maximum and suits up to 800k (nRF52840 product specification, SAADC acquisition time).
        constexpr uint8_t adc_sample_time_us = 40;
    }

    inline uint32_t read_battery_adc() {
        analogSampleTime(BatteryDivider::adc_sample_time_us);
        const auto adc_raw = analogRead(PIN_004);
        analogSampleTime(3);    // back to the default
        return adc_raw;
    }
    constexpr auto battery_division_factor =
        (BatteryDivider::r_top_ohm + BatteryDivider::r_bottom_ohm) / BatteryDivider::r_bottom_ohm;
}
