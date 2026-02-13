//
// Created by Paul Schroeder on 13.02.2026.
//

#pragma once

#include <SPI.h>

#include "Interfaces.hpp"

namespace Hardware {
    class _spi : public Interfaces::SPI{};
    _spi SPI;
}