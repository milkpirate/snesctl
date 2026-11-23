#include <catch_amalgamated.hpp>

#include "Utils.hpp"

using Catch::Approx;

TEST_CASE("map scales linearly") {
    CHECK(Utils::map(0, 0, 10, 0, 100) == 0);
    CHECK(Utils::map(5, 0, 10, 0, 100) == 50);
    CHECK(Utils::map(10, 0, 10, 0, 100) == 100);
    CHECK(Utils::map(15, 0, 10, 0, 100) == 150);     // not saturated
    CHECK(Utils::map(5, 0, 10, 100, 0) == 50);       // inverted output range
}

TEST_CASE("map_saturated limits to the output range") {
    // LiPo voltage to battery percentage, as used by the firmware
    const auto [voltage, percent] = GENERATE(table<float, float>({
        {2.0f,    0.0f},
        {2.8f,    0.0f},
        {3.5f,   50.0f},
        {4.2f,  100.0f},
        {5.0f,  100.0f},
    }));
    CAPTURE(voltage);

    CHECK(Utils::map_saturated(voltage, 2.8f, 4.2f, 0.0f, 100.0f) == Approx(percent));
}
