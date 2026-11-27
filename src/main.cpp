#include <Arduino.h>
#include <SPI.h>

#include <bluefruit.h>

#include <cmath>
#include <cstring>
#include <optional>

#include "Config.hpp"
#include "Interpolate.hpp"
#include "RadioSync.hpp"
#include "SNESctl.hpp"
#include "StatusLed.hpp"
#include "Debug.hpp"


BLEDis ble_dis;
BLEBas ble_bas;
BLEHidGamepad ble_gamepad;
SoftwareTimer battery_timer;
SoftwareTimer idle_timer;                   // connected, but no input: go to light sleep
SoftwareTimer wake_check_timer;             // in light sleep: check for a button press to wake up
SoftwareTimer light_sleep_timeout_timer;    // in light sleep for too long: power down completely

// SPIM2 instead of SPIM3: the controller is way slower than the 8MHz limit of SPIM0..2,
// and SPIM3 keeps drawing current after being disabled (nRF52840 anomaly 195)
SPIClass spi(
    NRF_SPIM2,
    Config::Pins::spi_miso,
    Config::Pins::spi_sck,
    Config::Pins::spi_mosi
);
SNESctl::Controller snes_controller(
    spi,
    Config::Pins::latch,
    Config::Pins::ctl_vcc,
    Config::Pins::ctl_gnd,
    Config::Board::ctl_vcc_on
);


volatile bool connected = false;
volatile bool light_sleep = false;  // disconnected on purpose (idle), not advertising
bool battery_sent = false;


void report_controller_state();
void setup_advertising();
void start_advertising();
void enter_light_sleep();
void wake_on_button_press();
void enter_system_off();
float read_battery_voltage();
uint8_t read_battery_percentage();


void setup() {
    Serial.begin(115200);
    analogReadResolution(Config::Battery::adc_resolution);
    analogOversampling(Config::Battery::adc_oversampling);

#ifdef WAIT_FOR_SERIAL
    // Blocking wait until the USB serial is opened, to not miss any output. Never use on battery!
    while ( !Serial ) {
        delay(10);
    }
#endif
    Serial.println("Nice!nano HID (SNES) Gamepad");
    Serial.println("----------------------------\n");

    Serial.println();
    Serial.println("Go to your phone's/laptop's Bluetooth settings to pair this device");
    Serial.println("then open an application that accepts gamepad input");
    dbgPrintVersion();
    dbgMemInfo();

    Bluefruit.begin();
    if constexpr (Config::Board::dcdc_enabled) {
        sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    }
    // read the controller right before every connection event, so changes go out with it
    RadioSync::begin(Config::Ble::radio_notification_distance, report_controller_state);
    Bluefruit.setTxPower(Config::Ble::tx_power_dbm);
    Bluefruit.autoConnLed(false);   // its LED pin is not connected on the nice!nano, see StatusLed instead
    StatusLed::begin(Config::Pins::status_led, LED_STATE_ON, Config::StatusLed::on_ms, Config::StatusLed::period_ms);
    Bluefruit.setName("SNESctl");

    // Configure and Start Device Information Service
    ble_dis.setManufacturer("DIY Industries");
    ble_dis.setModel("SNESctl");
    ble_dis.setFirmwareRev(FIRMWARE_VERSION);
    ble_dis.setHardwareRev(Config::Board::name);
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

    // preferred connection parameters (PPCP), see Config::Ble
    Bluefruit.Periph.setConnInterval(Config::Ble::conn_interval_min, Config::Ble::conn_interval_max);
    Bluefruit.Periph.printInfo();
    Bluefruit.Periph.setConnectCallback([](const uint16_t conn_handle){
        // centrals do not apply the preferred parameters on their own, so request them (nullptr -> PPCP)
        sd_ble_gap_conn_param_update(conn_handle, nullptr);
        connected = true;
        StatusLed::stopBlinking();
        idle_timer.start();
        Serial.println("Connected!");
        battery_sent = ble_bas.notify(read_battery_percentage());
    });
    Bluefruit.Periph.setDisconnectCallback([](const uint16_t _, const uint8_t reason){
        connected = false;
        idle_timer.stop();
        Serial.printf("Disconnected, reason: %d\n", reason);
        // the controller is not read anymore (see report_controller_state), so the light sleep can take over
        if (light_sleep) {
            wake_check_timer.start();
            light_sleep_timeout_timer.start();
        } else {
            start_advertising();
        }
    });

    spi.begin();
    pinMode(Config::Pins::spi_mosi, INPUT); // unused, but set to input to avoid driving the line

    snes_controller.begin();
    idle_timer.begin(Config::Power::idle_timeout_ms, [](TimerHandle_t){ enter_light_sleep(); }, nullptr, false);
    wake_check_timer.begin(Config::Power::wake_check_interval_ms, [](TimerHandle_t){ wake_on_button_press(); });
    light_sleep_timeout_timer.begin(
        Config::Power::light_sleep_timeout_ms, [](TimerHandle_t){ enter_system_off(); }, nullptr, false
    );
    setup_advertising();
    start_advertising();


    battery_timer.begin(Config::Timing::battery_interval_ms, [](TimerHandle_t timer){
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

    battery_timer.start();
}

void loop() {
    waitForEvent();
}

// called right before every radio event (see RadioSync): sends the controller state, if it changed
void report_controller_state() {
    static std::optional<hid_gamepad_report_t> last_sent;
    static bool was_connected = false;

    if (!connected) {
        was_connected = false;
        return;
    }
    if (!was_connected) {
        last_sent.reset();  // (re)connected: send the current state, whatever was sent before
        was_connected = true;
    }

    const auto report = snes_controller.getGamepadReport();
    const bool changed = !last_sent || std::memcmp(&*last_sent, &report, sizeof report) != 0;
    if (!changed) return;
    idle_timer.reset();     // input: restart the idle timeout

    // if sending failed, it is still a change and retried with the next radio event
    const auto sent = ble_gamepad.report(&report);
    if (sent) last_sent = report;

#ifdef DEBUG_HID_REPORT
    SNESctl::Controller::printReport(Serial, report);
    Serial.printf("HID report sent: %s\n", sent ? "true" : "false");
#endif
}

void setup_advertising() {
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_GAMEPAD);
    Bluefruit.Advertising.addService(ble_gamepad);
    Bluefruit.Advertising.addName();

    /* Advertising
     * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     * - Timeout for fast mode is 30 seconds
     *
     * For recommended advertising interval
     * https://developer.apple.com/library/content/qa/qa1931/_index.html
     */
    Bluefruit.Advertising.restartOnDisconnect(false);   // done in the disconnect callback, unless going to sleep
    Bluefruit.Advertising.setInterval(32, 244);         // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);           // number of seconds in fast mode

    // we do not want to drain the battery if left on pairing, so power down completely if there is no connection
    // in time (until power cycled). The controller is not read while advertising, so this can not interfere.
    Bluefruit.Advertising.setStopCallback(enter_system_off);
}

void start_advertising() {
    Bluefruit.Advertising.start(Config::Power::advertising_timeout_s);
    StatusLed::startBlinking();
}

// connected, but no input for too long: disconnect, then wait for a button press (see wake_on_button_press)
void enter_light_sleep() {
    // already disconnecting, or a late restart of the idle timer (by the last read before a disconnect)
    const auto already_disconnecting = !connected;
    const auto already_sleeping = light_sleep;
    if (already_disconnecting || already_sleeping) return;
    Serial.println("Idle, entering light sleep. Press any button to wake up.");
    light_sleep = true;
    Bluefruit.disconnect(Bluefruit.connHandle());
}

// periodically in light sleep: wake up on any button press (advertise to reconnect)
void wake_on_button_press() {
    const auto report = snes_controller.getGamepadReport();
    const bool pressed = report.buttons != 0 || report.x != 0 || report.y != 0;
    if (!pressed) return;

    Serial.println("Button pressed, waking up.");
    wake_check_timer.stop();
    light_sleep_timeout_timer.stop();
    light_sleep = false;
    start_advertising();
}

void enter_system_off() {
    Serial.println("Entering system off. Power cycle to wake up.");
    Serial.flush();
    StatusLed::off();   // pin states are kept in system off
    snes_controller.powerOff();
    sd_power_system_off();  // does not return, wakes up (= reboots) on reset or plugging in USB
}

float read_battery_voltage() {
    const auto adc_raw = Config::Board::read_battery_adc();
    return (
        static_cast<float>(adc_raw)
        * Config::Battery::adc_v_ref
        * Config::Board::battery_division_factor
    ) / Config::Battery::adc_scale;
}

uint8_t read_battery_percentage() {
    const auto percentage = Utils::interpolate(Config::Battery::lipo_curve, read_battery_voltage());
    return static_cast<uint8_t>(std::lround(percentage));
}
