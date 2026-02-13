#include <Arduino.h>
#include <SPI.h>
#include <bluefruit.h>

#include "Debug.hpp"
#include "Utils.hpp"
#include "SNESctl.hpp"
#include "Pin.hpp"
#include "SPI.hpp"


namespace {
    constexpr float adc_v_ref = 3.6f;
    constexpr uint8_t adc_resolution = 12;
    constexpr uint16_t adc_scale = (1u << adc_resolution) - 1u;
    constexpr float vddh_division_factor = 5.0f;

    constexpr float lipo_v_empty = 2.8f;
    constexpr float lipo_v_full = 4.2f;
}


auto latch = Hardware::Pin(PIN_002);


BLEDis ble_dis;
BLEBas ble_bas;
BLEHidGamepad ble_gamepad;
SoftwareTimer report_timer;
SNESctl::Controller snes_controller(Hardware::SPI,latch, Hardware::NC, Hardware::NC);


void start_advertising();
uint8_t read_battery_percentage();


void setup() {
    Serial.begin(115200);
    analogReadResolution(adc_resolution);

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

    SPI.begin();
    SPI.beginTransaction(SNESctl::Controller::get_spi_settings());

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
    Bluefruit.Periph.setConnectCallback([](const uint16_t _){ Serial.println("Connected!"); });
    Bluefruit.Periph.setDisconnectCallback([](const uint16_t _, const uint8_t reason){ Serial.printf("Disconnected, reason: %d\n", reason); });

    start_advertising();

    report_timer.begin(16, [](TimerHandle_t timer){
        static uint8_t last_battery_percent = ~0;

        auto hid_report = snes_controller.get_gamepad_report();
        Debug::print_binary(0, Serial);  // TODO: Add raw data method to Controller if needed
        const auto hid_report_sent = ble_gamepad.report(&hid_report);
        Serial.printf("HID Report sent: %s\n", hid_report_sent ? "true" : "false");

        auto battery_percent = read_battery_percentage();
        if (battery_percent != last_battery_percent) {
            const auto battery_sent = ble_bas.notify(battery_percent);
            Serial.printf("Battery %d%%, sent: %s\n", battery_percent, battery_sent ? "true" : "false");
            last_battery_percent = battery_percent;
        }
    });
    report_timer.start();
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

uint8_t read_battery_percentage() {
    // nice!nano v2 uses internal VDDH monitoring via analogReadVDDHDIV5()
    auto adc_raw = analogReadVDDHDIV5();
    auto voltage = (static_cast<float>(adc_raw) * adc_v_ref * vddh_division_factor) / adc_scale;
    // convert voltage to percentage (and saturate to the boundaries).
    // 2.8V..4.2V -> 0%..100%
    auto percentage = Utils::map_saturated(voltage, lipo_v_empty, lipo_v_full, 0.0f, 100.0f);
    return static_cast<uint8_t>(percentage);
}
