//
// Hardware independent decoding of the SNES controller shift register data (unit tested on the host).
//

#pragma once

#include <cstdint>


namespace SNESctl {
    struct GamepadState {
        uint32_t buttons;   // bit 0..7: B, Y, Select, Start, A, X, L, R
        int8_t x;           // D-pad left/right -> -127/127
        int8_t y;           // D-pad up/down -> -127/127 (origin -> upper left)
    };

    // `raw` as shifted out by the controller: active low, LSB first in the order
    // B, Y, Select, Start, Up, Down, Left, Right, A, X, L, R, (4x unused, always high)
    constexpr GamepadState decode(const uint16_t raw) {
        const uint16_t pressed = ~raw;   // inverted logic

        constexpr uint16_t sSYB_button_mask = 0b0000000000001111;   // Start, Select, Y, B
        constexpr uint16_t RLXA_button_mask = 0b0000111100000000;   // R, L, X, A
        constexpr uint16_t dpad_button_mask = 0b0000000011110000;   // D-pad: Right, Left, Down, Up

        const uint32_t buttons_sSYB = (pressed & sSYB_button_mask);
        const uint32_t buttons_RLXA = (pressed & RLXA_button_mask) >> 4;
        const uint8_t dpad          = (pressed & dpad_button_mask) >> 4;

        constexpr uint8_t up      = 1u << 0;
        constexpr uint8_t down    = 1u << 1;
        constexpr uint8_t left    = 1u << 2;
        constexpr uint8_t right   = 1u << 3;

        auto get_axis = [](const bool neg, const bool pos) -> int8_t {
            return static_cast<int8_t>((pos - neg) * 127);
        };

        return {
            .buttons = buttons_RLXA | buttons_sSYB,
            .x = get_axis(dpad & left, dpad & right),
            .y = get_axis(dpad & up,   dpad & down),
        };
    }
}
