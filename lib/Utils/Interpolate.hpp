//
// Piecewise linear interpolation over a table of points, e.g. a battery discharge curve (unit tested on the host).
//

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>

#include "Utils.hpp"


namespace Utils {
    template <typename T>
    struct Point {
        T x;
        T y;
    };

    // true if the points are sorted by x in ascending order (as required by `interpolate`)
    template <typename T, std::size_t N>
    constexpr bool is_sorted_by_x(const std::array<Point<T>, N>& points) {
        return std::is_sorted(points.begin(), points.end(), [](const auto& a, const auto& b) { return a.x < b.x; });
    }

    // y at `x` on the polyline through `points` (sorted by x), saturated to the first/last point outside of them
    template <typename T, std::size_t N>
    constexpr T interpolate(const std::array<Point<T>, N>& points, const T x) {
        static_assert(N >= 2, "at least two points are needed");

        if (x <= points.front().x) return points.front().y;
        if (x >= points.back().x) return points.back().y;

        // first point right of x, its predecessor is left of (or at) x
        const auto right = std::upper_bound(
            points.begin(), points.end(), x, [](const T value, const auto& point) { return value < point.x; }
        );
        const auto left = std::prev(right);
        return map(x, left->x, right->x, left->y, right->y);
    }
}
