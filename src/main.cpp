#include <Arduino.h>
#include <SPI.h>
#include <bluefruit.h>

#include <Debug.hpp>
#include <SNESCtrl.hpp>


BLEDis bledis;
BLEHidGamepad blegamepad;


void start_advertising();


volatile bool report_needed = false;


void setup() {
    Serial.begin(115200);
    while ( !Serial ) {
        delay(10);
    }

    Serial.println("Nice!nano HID (SNES) Gamepad");
    Serial.println("-------------------------------\n");

    Serial.println();
    Serial.println("Go to your phone's/laptop's Bluetooth settings to pair this device");
    Serial.println("then open an application that accepts gamepad input");

    SPI.begin();
    SPI.beginTransaction(SNESCtrl::get_spi_settings());

    Bluefruit.setName("SNESCtl");
    Bluefruit.begin();
    Bluefruit.setTxPower(4);    // check bluefruit.h for supported values

    // Configure and Start Device Information Service
    bledis.setManufacturer("DIY Industries");
    bledis.setModel("SNESctl");
    bledis.begin();

    /* Start BLE HID
     * Note: Apple requires BLE device must have min connection interval >= 20m
     * ( The smaller the connection interval the faster we could send data).
     * However for HID and MIDI device, Apple could accept min connection interval
     * up to 11.25 ms. Therefore, BLEHidAdafruit::begin() will try to set the min and max
     * connection interval to 11.25  ms and 15 ms respectively for best performance.
     */
    blegamepad.begin();

    /* Set connection interval (min, max) to your perferred value.
     * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
     * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms
     */
    /* Bluefruit.Periph.setConnInterval(9, 12); */
    Bluefruit.Periph.setConnInterval(13, 13);   // = 16.25ms <= 16.66ms = 60Hz (refresh rate)
    Bluefruit.Periph.printInfo();

    Bluefruit.Periph.setConnectCallback([](const uint16_t conn_hdl){
        Serial.println("Connected!");
        report_needed = true;
    });
    Bluefruit.Periph.setDisconnectCallback([](const uint16_t conn_hdl, const uint8_t reason){
        Serial.printf("Disconnected, reason: %d\n", reason);
    });

    Bluefruit.setEventCallback([](ble_evt_t* evt){
        switch (evt->header.evt_id) {
            case BLE_GATTS_EVT_HVN_TX_COMPLETE:
                report_needed = true;
                break;
        }
    });

    start_advertising();
}

void loop() {
    if (!report_needed || !Bluefruit.Periph.connected()) {
        return;
    }

    auto raw_button_pressed = SNESCtrl::read_raw(&SPI, PIN_002);
    Debug::print_binary(raw_button_pressed, Serial);
    auto hid_report = SNESCtrl::get_gamepad_report_from_raw(raw_button_pressed);
    blegamepad.report(&hid_report);
    report_needed = false;
    delay(250);
}

void start_advertising() {
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_GAMEPAD);
    Bluefruit.Advertising.addService(blegamepad);
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

