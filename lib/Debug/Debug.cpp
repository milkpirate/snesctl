//
// Created by Paul Schroeder on 07.02.2026.
//

#include "Debug.hpp"

namespace Debug {
    void send_test_buttons_presses(BLEHidGamepad &hid, Print &streamer) {
        streamer.println("No pressing buttons");
        hid_gamepad_report_t gp;
        hid.report(&gp);
        delay(1000);

        //------------- DPAD / HAT -------------//
        streamer.println("Hat/DPAD UP");
        gp.hat = GAMEPAD_HAT_UP;
        hid.report(&gp);
        delay(1000);

        streamer.println("Hat/DPAD UP RIGHT");
        gp.hat = GAMEPAD_HAT_UP_RIGHT;
        hid.report(&gp);
        delay(1000);

        streamer.println("Hat/DPAD RIGHT");
        gp.hat = GAMEPAD_HAT_RIGHT;
        hid.report(&gp);
        delay(1000);

        streamer.println("Hat/DPAD DOWN RIGHT");
        gp.hat = GAMEPAD_HAT_DOWN_RIGHT;
        hid.report(&gp);
        delay(1000);

        streamer.println("Hat/DPAD DOWN");
        gp.hat = GAMEPAD_HAT_DOWN;
        hid.report(&gp);
        delay(1000);

        streamer.println("Hat/DPAD DOWN LEFT");
        gp.hat = GAMEPAD_HAT_DOWN_LEFT;
        hid.report(&gp);
        delay(1000);

        streamer.println("Hat/DPAD LEFT");
        gp.hat = GAMEPAD_HAT_LEFT;
        hid.report(&gp);
        delay(1000);

        streamer.println("Hat/DPAD UP LEFT");
        gp.hat = GAMEPAD_HAT_UP_LEFT;
        hid.report(&gp);
        delay(1000);

        streamer.println("Hat/DPAD CENTER");
        gp.hat = GAMEPAD_HAT_CENTERED;
        hid.report(&gp);
        delay(1000);

        //------------- Joystick 1 -------------//

        streamer.println("Joystick 1 UP");
        gp.x = 0;
        gp.y = -127;
        hid.report(&gp);
        delay(1000);

        streamer.println("Joystick 1 DOWN");
        gp.x = 0;
        gp.y = 127;
        hid.report(&gp);
        delay(1000);

        streamer.println("Joystick 1 RIGHT");
        gp.x = 127;
        gp.y = 0;
        hid.report(&gp);
        delay(1000);

        streamer.println("Joystick 1 LEFT");
        gp.x = -127;
        gp.y = 0;
        hid.report(&gp);
        delay(1000);

        streamer.println("Joystick 1 CENTER");
        gp.x = 0;
        gp.y = 0;
        hid.report(&gp);
        delay(1000);


        //------------- Joystick 2 -------------//
        streamer.println("Joystick 2 UP");
        gp.z  = 0;
        gp.rz = 127;
        hid.report(&gp);
        delay(1000);

        streamer.println("Joystick 2 DOWN");
        gp.z  = 0;
        gp.rz = -127;
        hid.report(&gp);
        delay(1000);

        streamer.println("Joystick 2 RIGHT");
        gp.z  = 127;
        gp.rz = 0;
        hid.report(&gp);
        delay(1000);

        streamer.println("Joystick 2 LEFT");
        gp.z  = -127;
        gp.rz = 0;
        hid.report(&gp);
        delay(1000);

        streamer.println("Joystick 2 CENTER");
        gp.z  = 0;
        gp.rz = 0;
        hid.report(&gp);
        delay(1000);

        //------------- Analog Trigger 1 -------------//
        streamer.println("Analog Trigger 1 UP");
        gp.rx = 127;
        hid.report(&gp);
        delay(1000);

        streamer.println("Analog Trigger 1 DOWN");
        gp.rx = -127;
        hid.report(&gp);
        delay(1000);

        streamer.println("Analog Trigger 1 CENTER");
        gp.rx = 0;
        hid.report(&gp);
        delay(1000);

        //------------- Analog Trigger 2 -------------//
        streamer.println("Analog Trigger 2 UP");
        gp.ry = 127;
        hid.report(&gp);
        delay(1000);

        streamer.println("Analog Trigger 2 DOWN");
        gp.ry = -127;
        hid.report(&gp);
        delay(1000);

        streamer.println("Analog Trigger 2 CENTER");
        gp.ry = 0;
        hid.report(&gp);
        delay(1000);

        //------------- Buttons -------------//
        for (int i=0; i<16; ++i)
        {
            streamer.print("Pressing button "); streamer.println(i);
            gp.buttons = (1ul << i);
            hid.report(&gp);
            delay(1000);
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
        delay(1000);
    }
    void print_binary(const uint16_t raw_buttons, Print &streamer) {
        streamer.printf("[%010d] 0123456789ABCDEF\n", millis());
        streamer.printf("             ");
        for (int i=0; i<16; ++i) {
            streamer.print(bitRead(raw_buttons, i) ? "_" : "█");
        }
        streamer.println();
    }
}