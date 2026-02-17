//
// Created by Paul Schroeder on 07.02.2026.
//

#include "Debug.hpp"


namespace Debug {
    void send_test_buttons_presses(BLEHidGamepad &hid, const uint32_t delay_ms, Print &streamer) {
        streamer.println("No pressing buttons");
        hid_gamepad_report_t gp;
        hid.report(&gp);
        delay(delay_ms);

        //------------- DPAD / HAT -------------//
        streamer.println("Hat/DPAD UP");
        gp.hat = GAMEPAD_HAT_UP;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Hat/DPAD UP RIGHT");
        gp.hat = GAMEPAD_HAT_UP_RIGHT;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Hat/DPAD RIGHT");
        gp.hat = GAMEPAD_HAT_RIGHT;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Hat/DPAD DOWN RIGHT");
        gp.hat = GAMEPAD_HAT_DOWN_RIGHT;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Hat/DPAD DOWN");
        gp.hat = GAMEPAD_HAT_DOWN;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Hat/DPAD DOWN LEFT");
        gp.hat = GAMEPAD_HAT_DOWN_LEFT;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Hat/DPAD LEFT");
        gp.hat = GAMEPAD_HAT_LEFT;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Hat/DPAD UP LEFT");
        gp.hat = GAMEPAD_HAT_UP_LEFT;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Hat/DPAD CENTER");
        gp.hat = GAMEPAD_HAT_CENTERED;
        hid.report(&gp);
        delay(delay_ms);

        //------------- Joystick 1 -------------//

        streamer.println("Joystick 1 UP");
        gp.x = 0;
        gp.y = -127;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Joystick 1 DOWN");
        gp.x = 0;
        gp.y = 127;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Joystick 1 RIGHT");
        gp.x = 127;
        gp.y = 0;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Joystick 1 LEFT");
        gp.x = -127;
        gp.y = 0;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Joystick 1 CENTER");
        gp.x = 0;
        gp.y = 0;
        hid.report(&gp);
        delay(delay_ms);


        //------------- Joystick 2 -------------//
        streamer.println("Joystick 2 UP");
        gp.z  = 0;
        gp.rz = 127;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Joystick 2 DOWN");
        gp.z  = 0;
        gp.rz = -127;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Joystick 2 RIGHT");
        gp.z  = 127;
        gp.rz = 0;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Joystick 2 LEFT");
        gp.z  = -127;
        gp.rz = 0;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Joystick 2 CENTER");
        gp.z  = 0;
        gp.rz = 0;
        hid.report(&gp);
        delay(delay_ms);

        //------------- Analog Trigger 1 -------------//
        streamer.println("Analog Trigger 1 UP");
        gp.rx = 127;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Analog Trigger 1 DOWN");
        gp.rx = -127;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Analog Trigger 1 CENTER");
        gp.rx = 0;
        hid.report(&gp);
        delay(delay_ms);

        //------------- Analog Trigger 2 -------------//
        streamer.println("Analog Trigger 2 UP");
        gp.ry = 127;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Analog Trigger 2 DOWN");
        gp.ry = -127;
        hid.report(&gp);
        delay(delay_ms);

        streamer.println("Analog Trigger 2 CENTER");
        gp.ry = 0;
        hid.report(&gp);
        delay(delay_ms);

        //------------- Buttons -------------//
        for (int i=0; i<16; ++i)
        {
            streamer.print("Pressing button "); streamer.println(i);
            gp.buttons = (1ul << i);
            hid.report(&gp);
            delay(delay_ms);
        }

        // Random touch
        streamer.println("Random touch");

        gp.x       = (int8_t)random(-127, 128);
        gp.y       = (int8_t)random(-127, 128);
        gp.z       = (int8_t)random(-127, 128);
        gp.rz      = (int8_t)random(-127, 128);
        gp.rx      = (int8_t)random(-127, 128);
        gp.ry      = (int8_t)random(-127, 128);
        gp.hat     = random(0, 9);
        gp.buttons = random(0, 0xffff);

        hid.report(&gp);
        delay(delay_ms);
    }
    void print_gatt_event(const uint16_t id, Print &streamer) {
        switch (id) {
            case BLE_GATTS_EVT_WRITE:
                streamer.println("BLE_GATTS_EVT_WRITE, Write operation performed.");
                break;
            case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
                streamer.println("BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST, Read/Write Authorization request.");
                break;
            case BLE_GATTS_EVT_SYS_ATTR_MISSING:
                streamer.println("BLE_GATTS_EVT_SYS_ATTR_MISSING, A persistent system attribute access is pending.");
                break;
            case BLE_GATTS_EVT_HVC:
                streamer.println("BLE_GATTS_EVT_HVC, Handle Value Confirmation.");
                break;
            case BLE_GATTS_EVT_SC_CONFIRM:
                streamer.println("BLE_GATTS_EVT_SC_CONFIRM, Service Changed Confirmation.");
                break;
            case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
                streamer.println("BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST, Exchange MTU Request.");
                break;
            case BLE_GATTS_EVT_TIMEOUT:
                streamer.println("BLE_GATTS_EVT_TIMEOUT, Peer failed to respond to an ATT request in time.");
                break;
            case BLE_GATTS_EVT_HVN_TX_COMPLETE:
                streamer.println("BLE_GATTS_EVT_HVN_TX_COMPLETE, Handle Value Notification transmission complete.");
                break;
            default:
                streamer.printf("Unknown event ID: %d\n", id);
        }
    };
}