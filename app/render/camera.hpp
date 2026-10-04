#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

namespace genesis {

// Экранных пикселей на клетку
inline constexpr float kMinZoom = 1.0f;
inline constexpr float kMaxZoom = 128.0f;

// Мировые координаты измеряются в клетках, поэтому любой слой карты проецируется
// в один и тот же прямоугольник независимо от разрешения его текстуры
struct Camera {
    float x = 0.0f;  // мировые координаты левого верхнего угла экрана
    float y = 0.0f;
    float zoom = 1.0f;
};

SDL_FPoint screenToWorld(const Camera& camera, SDL_FPoint screen);
SDL_FRect worldToScreen(const Camera& camera, SDL_FRect world);

// Зум, при котором карта целиком помещается в область экрана area
float fitZoom(int32_t mapWidth, int32_t mapHeight, const SDL_FRect& area);

// Карта целиком в area, по её центру
Camera fitCamera(int32_t mapWidth, int32_t mapHeight, const SDL_FRect& area);

// Не даёт отдалиться дальше всей карты и увести её из area: по оси, где карта меньше области,
// она стоит по центру, где больше, её край не заходит внутрь области
void clampToMap(Camera& camera, int32_t mapWidth, int32_t mapHeight, const SDL_FRect& area);

// dx, dy в экранных пикселях
void pan(Camera& camera, float dx, float dy);

// Мировая точка под screen остаётся на месте
void zoomAt(Camera& camera, SDL_FPoint screen, float factor);

// Индекс клетки под точкой экрана или -1
int32_t pickCell(const Camera& camera, SDL_FPoint screen, int32_t mapWidth, int32_t mapHeight);

} // namespace genesis
