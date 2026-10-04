#pragma once

#include <cstdint>
#include <vector>

namespace genesis {

struct WorldSpec {
    int32_t width = 100;
    int32_t height = 100;
    double waterFraction = 0.40;
};

struct World {
    WorldSpec spec;
    uint64_t seed = 0;
    std::vector<float> elevation;  // [0, 1]
    float seaLevel = 0.0f;         // ниже лежит доля spec.waterFraction клеток
};

} // namespace genesis
