#include <Arduino.h>
#include <SPI.h>

#include <bluefruit.h>

#include "Utils.hpp"
#include "SNESctl.hpp"
#include "Debug.hpp"

namespace Config {
    namespace Battery {
        constexpr auto adc_v_ref = 3.6f;
        constexpr uint8_t adc_resolution = 12;
        constexpr uint16_t adc_scale = (1u << adc_resolution) - 1u;
        constexpr auto vddh_division_factor = 5.0f;

        constexpr auto lipo_v_empty = 2.8f;
        constexpr auto lipo_v_full = 4.2f;
    }

    namespace Pins {
        constexpr auto spi_mosi = PIN_113;  // unused

        constexpr auto spi_sck = PIN_031;   // blue
        constexpr auto spi_miso = PIN_002;  // red
        constexpr auto latch = PIN_029;     // yellow

        constexpr auto ctl_vcc = PIN_013;   // white
        constexpr auto ctl_gnd = PIN_115;   // brown
    }
}


BLEDis ble_dis;
BLEBas ble_bas;
BLEHidGamepad ble_gamepad;
SoftwareTimer report_timer;
SoftwareTimer battery_timer;

SPIClass spi(
    NRF_SPIM3,
    Config::Pins::spi_miso,
    Config::Pins::spi_sck,
    Config::Pins::spi_mosi
);
SNESctl::Controller snes_controller(
    spi,
    Config::Pins::latch,
    Config::Pins::ctl_vcc,
    Config::Pins::ctl_gnd
);


bool connected = false;
bool battery_sent = false;


void start_advertising();
float read_battery_voltage();
uint8_t read_battery_percentage();


void setup() {
    Serial.begin(115200);
    analogReadResolution(Config::Battery::adc_resolution);

    // Blocking wait for connection when debug mode is enabled via IDE
    while ( !Serial ) {
        delay(10);
    }
    Serial.println("Nice!nano HID (SNES) Gamepad");
    Serial.println("----------------------------\n");

    Serial.println();
    Serial.println("Go to your phone's/laptop's Bluetooth settings to pair this device");
    Serial.println("then open an application that accepts gamepad input");
    dbgPrintVersion();
    dbgMemInfo();

    Bluefruit.begin();
    Bluefruit.setTxPower(4);
    Bluefruit.setName("SNESctl");

    // Configure and Start Device Information Service
    ble_dis.setManufacturer("DIY Industries");
    ble_dis.setModel("SNESctl");
    ble_dis.begin();

    // Configure and Start Battery Service
    ble_bas.begin();

    /* Start BLE HID
     * Note: Apple requires BLE device must have min connection interval >= 20m
     * ( The smaller the connection interval the faster we could send data).
     * However for HID and MIDI device, Apple could accept min connection interval
     * up to 11.25 ms. Therefore, BLEHidAdafruit::begin() will try to set the min and max
     * connection interval to 11.25  ms and 15 ms respectively for best performance.
     */
    ble_gamepad.begin();

    /* Set connection interval (min, max) to your perferred value.
     * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
     * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms
     */
    Bluefruit.Periph.setConnInterval(9, 12);
    Bluefruit.Periph.printInfo();
    Bluefruit.Periph.setConnectCallback([](const uint16_t _){
        connected = true;
        Serial.println("Connected!");
        battery_sent = ble_bas.notify(read_battery_percentage());
    });
    Bluefruit.Periph.setDisconnectCallback([](const uint16_t _, const uint8_t reason){
        connected = false;
        Serial.printf("Disconnected, reason: %d\n", reason);
    });

    spi.begin();
    pinMode(Config::Pins::spi_mosi, INPUT); // unused, but set to input to avoid driving the line

    snes_controller.begin();
    start_advertising();


    report_timer.begin(16, [](TimerHandle_t timer){
        hid_gamepad_report_t hid_report = snes_controller.getGamepadReport();
        const auto hid_report_sent = ble_gamepad.report(&hid_report);
#ifdef DEBUG_HID_REPORT
        SNESctl::Controller::printRawButtons(Serial, snes_controller.readRaw());
        SNESctl::Controller::printReport(Serial, hid_report);
        Serial.printf(
                "HID report sent: %s, connected %s\n",
                hid_report_sent ? "true" : "false",
                connected ? "true" : "false"
        );
#endif
    });

    battery_timer.begin(60 * 1000, [](TimerHandle_t timer){
        static uint8_t last_battery_percent = ~0;
        auto battery_percent = read_battery_percentage();
        // send update if percentage changed, or we haven't sent the current percentage yet (e.g. on first connection)
        if (battery_percent == last_battery_percent && battery_sent) return;
        battery_sent = ble_bas.notify(battery_percent);
        last_battery_percent = battery_percent;

#ifdef DEBUG_BATTERY_REPORT
        Serial.printf(
            "Battery %d%% (%.02fV), sent: %s, connected %s\n",
            battery_percent,
            read_battery_voltage(),
            battery_sent ? "true" : "false",
            connected ? "true" : "false"
        );
#endif
    });

    report_timer.start();
    battery_timer.start();
}

void loop() {
    waitForEvent();
}

void start_advertising() {
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_GAMEPAD);
    Bluefruit.Advertising.addService(ble_gamepad);
    Bluefruit.Advertising.addName();

    /* Start Advertising
     * - Enable auto advertising if disconnected
     * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     * - Timeout for fast mode is 30 seconds
     * - Start(timeout) with timeout = 0 will advertise forever (until connected)
     *
     * For recommended advertising interval
     * https://developer.apple.com/library/content/qa/qa1931/_index.html
     */
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244);  // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);          // number of seconds in fast mode

    // stop advertising after 2mins (0 => do not stop)
    // we do not want to drain the battery if left on pairing
    Bluefruit.Advertising.start(120);
}

float read_battery_voltage() {
    // nice!nano v2 uses internal VDDH monitoring via analogReadVDDHDIV5()
    auto adc_raw = analogReadVDDHDIV5();
    return (
        static_cast<float>(adc_raw)
        * Config::Battery::adc_v_ref
        * Config::Battery::vddh_division_factor
    ) / Config::Battery::adc_scale;
}

uint8_t read_battery_percentage() {
    auto voltage = read_battery_voltage();
    // convert voltage to percentage (and saturate to the boundaries).
    // 2.8V..4.2V -> 0%..100%
    auto percentage = Utils::map_saturated(
        voltage,
        Config::Battery::lipo_v_empty,
        Config::Battery::lipo_v_full,
        0.0f,
        100.0f
    );
    return static_cast<uint8_t>(percentage);
}
