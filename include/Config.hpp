//
// Firmware configuration. Board revision specific parts are in config/<env>/BoardConfig.hpp,
// selected by the include path of the PlatformIO environment (see platformio.ini).
//

#pragma once

#include <Arduino.h>
#include <nrf_soc.h>

#include <array>

#include "BoardConfig.hpp"
#include "Interpolate.hpp"


namespace Config {
    namespace Timing {
        constexpr uint32_t battery_interval_ms = 60 * 1000;
    }

    namespace Power {
        // waiting for a connection: power down completely (system off) after this, to not drain the battery
        constexpr uint16_t advertising_timeout_s = 2 * 60;
        // connected, but no input: disconnect and go to light sleep, waking up on any button press
        constexpr uint32_t idle_timeout_ms = 15 * 60 * 1000;
        // in light sleep: how often the controller is checked for a button press to wake up (only in light sleep,
        // not while connected)
        constexpr uint32_t wake_check_interval_ms = 100;
        // in light sleep without a button press for this long: power down completely (system off)
        constexpr uint32_t light_sleep_timeout_ms = 60 * 60 * 1000;
    }

    namespace StatusLed {
        // short flash while waiting for a connection, off otherwise
        constexpr uint32_t on_ms = 50;
        constexpr uint32_t period_ms = 1000;
    }

    namespace Ble {
        // transmit power in dBm (-40, -20, -16, -12, -8, -4, 0, 2..8). Lower saves power while sending,
        // 0dBm still easily covers a room.
        constexpr int8_t tx_power_dbm = 0;

        // preferred connection parameters, requested from the central after connecting. The controller is read
        // right before every connection event (see RadioSync), so the interval is also the polling rate, similar
        // to the SNES (~60Hz). There is no slave latency on purpose: skipped connection events would delay reading.
        constexpr uint16_t conn_interval_min = 9;   // * 1.25ms = 11.25ms (the minimum Apple accepts for HID)
        constexpr uint16_t conn_interval_max = 12;  // * 1.25ms = 15ms

        // how long before a radio event the controller is read. Reading takes ~0.2ms, the report has to be queued
        // before the event starts. Fixed steps of the SoftDevice: 0.8, 1.74, 2.68, 3.62, 4.56 or 5.5ms
        constexpr uint8_t radio_notification_distance = NRF_RADIO_NOTIFICATION_DISTANCE_1740US;
    }

    namespace Battery {
        // ADC input range: internal 0.6V reference with gain 1/6 (framework default) -> 0..3.6V
        constexpr auto adc_v_ref = 3.6f;
        constexpr uint8_t adc_resolution = 12;
        constexpr uint16_t adc_scale = (1u << adc_resolution) - 1u;
        // samples averaged in hardware per reading, to reduce noise
        constexpr uint8_t adc_oversampling = 8;

        // typical state of charge (%) of a single LiPo cell by its voltage (V), at light load
        constexpr std::array<Utils::Point<float>, 21> lipo_curve{{
            {3.27f,   0.0f},
            {3.61f,   5.0f},
            {3.69f,  10.0f},
            {3.71f,  15.0f},
            {3.73f,  20.0f},
            {3.75f,  25.0f},
            {3.77f,  30.0f},
            {3.79f,  35.0f},
            {3.80f,  40.0f},
            {3.82f,  45.0f},
            {3.84f,  50.0f},
            {3.85f,  55.0f},
            {3.87f,  60.0f},
            {3.91f,  65.0f},
            {3.95f,  70.0f},
            {3.98f,  75.0f},
            {4.02f,  80.0f},
            {4.08f,  85.0f},
            {4.11f,  90.0f},
            {4.15f,  95.0f},
            {4.20f, 100.0f},
        }};
        static_assert(Utils::is_sorted_by_x(lipo_curve), "lipo_curve must be sorted by voltage");
    }

    namespace Pins {
        constexpr auto spi_mosi = PIN_113;  // unused

        constexpr auto spi_sck = PIN_031;   // blue
        constexpr auto spi_miso = PIN_002;  // red
        constexpr auto latch = PIN_029;     // yellow

        constexpr auto ctl_vcc = PIN_013;   // white
        constexpr auto ctl_gnd = PIN_115;   // brown

        constexpr auto status_led = LED_BUILTIN;    // P0.15
    }
}
