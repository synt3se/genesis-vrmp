#pragma once

#include <cstdint>

#include "genesis/world.hpp"

namespace genesis {

struct Rgb {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

// Параметры отрисовки: меняются без перегенерации мира
struct MapStyle {
    float reliefExaggeration = 30.0f;
    float ambient = 0.35f;
};

Rgb terrainColor(const World& world, int32_t cell, const MapStyle& style);

} // namespace genesis
