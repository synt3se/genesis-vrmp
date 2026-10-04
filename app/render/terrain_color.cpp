#include "terrain_color.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <span>

#include "genesis/grid.hpp"

namespace genesis {

namespace {

struct ColorStop {
    float t = 0.0f;
    Rgb color;
};

// t = 0 у берега, 1 на максимальной глубине
constexpr std::array<ColorStop, 3> kWaterStops{{
    {0.00f, {104, 178, 204}},
    {0.35f, {56, 128, 178}},
    {1.00f, {22, 54, 104}},
}};

// t = 0 у берега, 1 на максимальной высоте; шкала физической карты
constexpr std::array<ColorStop, 7> kLandStops{{
    {0.00f, {226, 212, 164}},
    {0.05f, {148, 184, 104}},
    {0.30f, {92, 146, 76}},
    {0.55f, {156, 150, 98}},
    {0.75f, {136, 116, 96}},
    {0.90f, {176, 172, 166}},
    {1.00f, {246, 246, 246}},
}};

// Свет с северо-запада под 45 градусов к горизонту, вектор единичной длины
constexpr float kLightX = -0.5f;
constexpr float kLightY = -0.5f;
constexpr float kLightZ = 0.70710678f;

uint8_t lerpChannel(uint8_t a, uint8_t b, float k) {
    return static_cast<uint8_t>(std::lerp(static_cast<float>(a), static_cast<float>(b), k) + 0.5f);
}

Rgb sampleGradient(std::span<const ColorStop> stops, float t) {
    const float clamped = std::clamp(t, 0.0f, 1.0f);
    for (size_t i = 1; i < stops.size(); ++i) {
        if (clamped <= stops[i].t) {
            const ColorStop& a = stops[i - 1];
            const ColorStop& b = stops[i];
            const float k = (clamped - a.t) / (b.t - a.t);
            return Rgb{lerpChannel(a.color.r, b.color.r, k), lerpChannel(a.color.g, b.color.g, k),
                       lerpChannel(a.color.b, b.color.b, k)};
        }
    }
    return stops.back().color;
}

float elevationAt(const World& world, int32_t x, int32_t y) {
    const int32_t cx = std::clamp(x, 0, world.spec.width - 1);
    const int32_t cy = std::clamp(y, 0, world.spec.height - 1);
    return world.elevation[static_cast<size_t>(idx(cx, cy, world.spec.width))];
}

// Ламберт, нормированный так, что горизонталь даёт 1: равнина сохраняет цвет шкалы
float hillshade(const World& world, int32_t x, int32_t y, const MapStyle& style) {
    const float k = 0.5f * style.reliefExaggeration;
    const float dzdx = (elevationAt(world, x + 1, y) - elevationAt(world, x - 1, y)) * k;
    const float dzdy = (elevationAt(world, x, y + 1) - elevationAt(world, x, y - 1)) * k;
    // Нормаль к поверхности z = h(x, y) направлена как (-dzdx, -dzdy, 1)
    const float invLength = 1.0f / std::sqrt(dzdx * dzdx + dzdy * dzdy + 1.0f);
    const float lambert = std::max(0.0f, (-dzdx * kLightX - dzdy * kLightY + kLightZ) * invLength);
    const float flat = style.ambient + (1.0f - style.ambient) * kLightZ;
    return (style.ambient + (1.0f - style.ambient) * lambert) / flat;
}

uint8_t scaleChannel(uint8_t c, float k) {
    return static_cast<uint8_t>(std::min(255.0f, static_cast<float>(c) * k + 0.5f));
}

} // namespace

Rgb terrainColor(const World& world, int32_t cell, const MapStyle& style) {
    const float h = world.elevation[static_cast<size_t>(cell)];
    const float sea = world.seaLevel;
    if (h < sea) {
        // Поверхность воды плоская, поэтому без затенения
        return sampleGradient(kWaterStops, (sea - h) / sea);
    }
    const Rgb base = sampleGradient(kLandStops, (h - sea) / (1.0f - sea));
    const float shade = hillshade(world, cellX(cell, world.spec.width), cellY(cell, world.spec.width), style);
    return Rgb{scaleChannel(base.r, shade), scaleChannel(base.g, shade), scaleChannel(base.b, shade)};
}

} // namespace genesis
