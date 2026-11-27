#include <catch_amalgamated.hpp>

#include "Utils.hpp"

TEST_CASE("map scales linearly") {
    CHECK(Utils::map(0, 0, 10, 0, 100) == 0);
    CHECK(Utils::map(5, 0, 10, 0, 100) == 50);
    CHECK(Utils::map(10, 0, 10, 0, 100) == 100);
    CHECK(Utils::map(15, 0, 10, 0, 100) == 150);     // not saturated
    CHECK(Utils::map(5, 0, 10, 100, 0) == 50);       // inverted output range
}
