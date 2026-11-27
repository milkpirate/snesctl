//
// Created by Paul Schroeder on 10.02.2026.
//

#pragma once

#include <type_traits>


namespace Utils {
    template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    constexpr T map(T value, T in_min, T in_max, T out_min, T out_max) {
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
}