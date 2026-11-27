//
// Identify the nice!nano revision: `task probe`, then watch the serial output (`task monitor`).
//
// v2: the battery is connected to VDDH, so the nRF52840 runs in high voltage mode
// v1: the nRF52840 is supplied via VDD (normal voltage mode), the battery is measured via a divider on P0.04
//

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>  // USB serial

namespace {
    constexpr auto adc_v_ref = 3.6f;
    constexpr auto adc_scale = 4095.0f;

    float to_volts(const uint32_t adc_raw, const float division_factor = 1.0f) {
        return static_cast<float>(adc_raw) * adc_v_ref * division_factor / adc_scale;
    }
}

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
}

void loop() {
    const bool high_voltage_mode =
        (NRF_POWER->MAINREGSTATUS & POWER_MAINREGSTATUS_MAINREGSTATUS_Msk) == POWER_MAINREGSTATUS_MAINREGSTATUS_High;

    const auto vddh = to_volts(analogReadVDDHDIV5(), 5.0f);
    const auto vdd = to_volts(analogReadVDD());
    analogSampleTime(40);   // high impedance v1 divider, see config/nicenano_v1/BoardConfig.hpp
    const auto p004 = to_volts(analogRead(PIN_004), (806e3f + 2e6f) / 2e6f);
    analogSampleTime(3);

    Serial.printf(
        "regulator: %s | VDDH: %.2fV | VDD: %.2fV | P0.04 (v1 battery divider): %.2fV => looks like %s\n",
        high_voltage_mode ? "high voltage mode (VDDH)" : "normal voltage mode (VDD)",
        vddh, vdd, p004,
        high_voltage_mode ? "v2" : "v1"
    );
    delay(1000);
}
