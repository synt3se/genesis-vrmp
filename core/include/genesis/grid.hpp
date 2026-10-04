#pragma once

#include <cstdint>

namespace genesis {

// Все поля мира хранятся плоскими массивами по строкам
constexpr int32_t idx(int32_t x, int32_t y, int32_t w) {
    return y * w + x;
}

constexpr int32_t cellX(int32_t i, int32_t w) {
    return i % w;
}

constexpr int32_t cellY(int32_t i, int32_t w) {
    return i / w;
}

constexpr bool inBounds(int32_t x, int32_t y, int32_t w, int32_t h) {
    return x >= 0 && y >= 0 && x < w && y < h;
}

} // namespace genesis
