#include <catch_amalgamated.hpp>

#include <array>

#include "Interpolate.hpp"

using Catch::Approx;
using Utils::Point;

namespace {
    // excerpt of a LiPo discharge curve (voltage -> state of charge), non-uniformly spaced like the real one
    constexpr std::array<Point<float>, 5> curve{{
        {3.27f,   0.0f},
        {3.61f,   5.0f},
        {3.84f,  50.0f},
        {4.02f,  80.0f},
        {4.20f, 100.0f},
    }};
}

TEST_CASE("interpolate hits the points exactly") {
    for (const auto& point : curve) {
        CAPTURE(point.x);
        CHECK(Utils::interpolate(curve, point.x) == Approx(point.y));
    }
}

TEST_CASE("interpolate is linear between points") {
    CHECK(Utils::interpolate(curve, 3.44f) == Approx(2.5f));     // halfway between 0% and 5%
    CHECK(Utils::interpolate(curve, 3.93f) == Approx(65.0f));    // halfway between 50% and 80%
    CHECK(Utils::interpolate(curve, 4.11f) == Approx(90.0f));    // halfway between 80% and 100%
}

TEST_CASE("interpolate saturates outside of the points") {
    CHECK(Utils::interpolate(curve, 2.5f) == Approx(0.0f));
    CHECK(Utils::interpolate(curve, 4.35f) == Approx(100.0f));   // e.g. while charging
}

TEST_CASE("interpolate is monotonic for a monotonic curve") {
    float last = -1.0f;
    for (float volts = 3.0f; volts <= 4.3f; volts += 0.01f) {
        const auto percent = Utils::interpolate(curve, volts);
        CAPTURE(volts);
        CHECK(percent >= last);
        last = percent;
    }
}

TEST_CASE("interpolate works with two points") {
    constexpr std::array<Point<int>, 2> line{{{0, 0}, {10, 100}}};
    CHECK(Utils::interpolate(line, 5) == 50);
    CHECK(Utils::interpolate(line, -1) == 0);
    CHECK(Utils::interpolate(line, 11) == 100);
}

TEST_CASE("is_sorted_by_x and interpolate are usable at compile time") {
    STATIC_CHECK(Utils::is_sorted_by_x(curve));
    constexpr std::array<Point<int>, 2> unsorted{{{10, 0}, {0, 100}}};
    STATIC_CHECK_FALSE(Utils::is_sorted_by_x(unsorted));
    constexpr std::array<Point<int>, 2> line{{{0, 0}, {10, 100}}};
    STATIC_CHECK(Utils::interpolate(line, 3) == 30);
}
