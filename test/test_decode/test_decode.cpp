#include <catch_amalgamated.hpp>

#include "SNESdecode.hpp"

using SNESctl::decode;

namespace {
    // raw shift register value with the given bits pressed (active low)
    constexpr uint16_t pressed(const uint16_t bits) {
        return static_cast<uint16_t>(~bits);
    }

    namespace Raw {
        constexpr uint16_t B      = 1u << 0;
        constexpr uint16_t Y      = 1u << 1;
        constexpr uint16_t Select = 1u << 2;
        constexpr uint16_t Start  = 1u << 3;
        constexpr uint16_t Up     = 1u << 4;
        constexpr uint16_t Down   = 1u << 5;
        constexpr uint16_t Left   = 1u << 6;
        constexpr uint16_t Right  = 1u << 7;
        constexpr uint16_t A      = 1u << 8;
        constexpr uint16_t X      = 1u << 9;
        constexpr uint16_t L      = 1u << 10;
        constexpr uint16_t R      = 1u << 11;
    }
}

TEST_CASE("nothing pressed") {
    const auto state = decode(0xFFFF);
    CHECK(state.buttons == 0);
    CHECK(state.x == 0);
    CHECK(state.y == 0);
}

TEST_CASE("unused bits are ignored") {
    // genuine controllers shift out 1s, some knock offs might not
    const auto state = decode(0x0FFF);
    CHECK(state.buttons == 0);
    CHECK(state.x == 0);
    CHECK(state.y == 0);
}

TEST_CASE("buttons map to HID buttons 0..7") {
    const auto [raw, hid_button] = GENERATE(table<uint16_t, uint32_t>({
        {Raw::B,      0},
        {Raw::Y,      1},
        {Raw::Select, 2},
        {Raw::Start,  3},
        {Raw::A,      4},
        {Raw::X,      5},
        {Raw::L,      6},
        {Raw::R,      7},
    }));
    CAPTURE(raw, hid_button);

    const auto state = decode(pressed(raw));
    CHECK(state.buttons == (1u << hid_button));
    CHECK(state.x == 0);
    CHECK(state.y == 0);
}

TEST_CASE("all buttons pressed") {
    const auto state = decode(pressed(
        Raw::B | Raw::Y | Raw::Select | Raw::Start | Raw::A | Raw::X | Raw::L | Raw::R
    ));
    CHECK(state.buttons == 0xFF);
}

TEST_CASE("D-pad maps to X/Y axis") {
    const auto [raw, x, y] = GENERATE(table<uint16_t, int8_t, int8_t>({
        {Raw::Up,                   0, -127},
        {Raw::Down,                 0,  127},
        {Raw::Left,              -127,    0},
        {Raw::Right,              127,    0},
        {Raw::Up | Raw::Left,    -127, -127},
        {Raw::Down | Raw::Right,  127,  127},
        // opposing directions cancel out (worn out D-pads)
        {Raw::Up | Raw::Down,       0,    0},
        {Raw::Left | Raw::Right,    0,    0},
    }));
    CAPTURE(raw);

    const auto state = decode(pressed(raw));
    CHECK(state.buttons == 0);
    CHECK(state.x == x);
    CHECK(state.y == y);
}

TEST_CASE("decode is usable at compile time") {
    STATIC_CHECK(decode(pressed(Raw::Start)).buttons == (1u << 3));
}
