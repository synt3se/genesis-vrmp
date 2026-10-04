#include "test_world.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "FastNoiseLite.h"

#include "genesis/grid.hpp"

namespace genesis {

namespace {

constexpr float kNoiseFrequency = 0.035f;
constexpr int kNoiseOctaves = 5;

// Расстояние от центра в долях полуширины карты: до kIslandInner суша не опускается,
// к kIslandOuter уходит под воду полностью, поэтому край карты всегда море
constexpr float kIslandInner = 0.45f;
constexpr float kIslandOuter = 1.0f;

float smoothstep(float edge0, float edge1, float x) {
    const float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

void normalize(std::vector<float>& values) {
    const auto [lo, hi] = std::minmax_element(values.begin(), values.end());
    const float min = *lo;
    const float range = *hi - *lo;
    if (range <= 0.0f) {
        return;
    }
    for (float& v : values) {
        v = (v - min) / range;
    }
}

// Копия намеренно: nth_element переставляет элементы
float waterThreshold(std::vector<float> elevation, double fraction) {
    const size_t k = std::min(elevation.size() - 1,
                              static_cast<size_t>(fraction * static_cast<double>(elevation.size())));
    std::nth_element(elevation.begin(), elevation.begin() + static_cast<std::ptrdiff_t>(k), elevation.end());
    return elevation[k];
}

} // namespace

World makeTestWorld(const WorldSpec& spec, uint64_t seed) {
    World world;
    world.spec = spec;
    world.seed = seed;
    const int32_t w = world.spec.width;
    const int32_t h = world.spec.height;
    world.elevation.assign(static_cast<size_t>(w) * h, 0.0f);

    FastNoiseLite noise(static_cast<int>(seed));
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(kNoiseOctaves);
    noise.SetFrequency(kNoiseFrequency);

    for (int32_t y = 0; y < h; ++y) {
        for (int32_t x = 0; x < w; ++x) {
            const float fx = static_cast<float>(x);
            const float fy = static_cast<float>(y);
            const float nx = 2.0f * fx / static_cast<float>(w - 1) - 1.0f;
            const float ny = 2.0f * fy / static_cast<float>(h - 1) - 1.0f;
            const float island = 1.0f - smoothstep(kIslandInner, kIslandOuter, std::sqrt(nx * nx + ny * ny));
            const float relief = 0.5f * (noise.GetNoise(fx, fy) + 1.0f);
            world.elevation[idx(x, y, w)] = relief * island;
        }
    }
    normalize(world.elevation);
    world.seaLevel = waterThreshold(world.elevation, world.spec.waterFraction);
    return world;
}

} // namespace genesis
