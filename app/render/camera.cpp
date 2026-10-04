#include "camera.hpp"

#include <algorithm>
#include <cmath>

#include "genesis/grid.hpp"

namespace genesis {

namespace {

// Положение камеры по одной оси при уже выбранном зуме
float clampAxis(float position, float zoom, float mapSize, float areaOffset, float areaSize) {
    const float mapOnScreen = mapSize * zoom;
    if (mapOnScreen <= areaSize) {
        return -(areaOffset + (areaSize - mapOnScreen) / 2.0f) / zoom;
    }
    const float nearEdge = -areaOffset / zoom;
    const float farEdge = mapSize - (areaOffset + areaSize) / zoom;
    return std::clamp(position, nearEdge, farEdge);
}

} // namespace

SDL_FPoint screenToWorld(const Camera& camera, SDL_FPoint screen) {
    return SDL_FPoint{screen.x / camera.zoom + camera.x, screen.y / camera.zoom + camera.y};
}

SDL_FRect worldToScreen(const Camera& camera, SDL_FRect world) {
    return SDL_FRect{(world.x - camera.x) * camera.zoom, (world.y - camera.y) * camera.zoom,
                     world.w * camera.zoom, world.h * camera.zoom};
}

float fitZoom(int32_t mapWidth, int32_t mapHeight, const SDL_FRect& area) {
    const float zoom = std::min(area.w / static_cast<float>(mapWidth), area.h / static_cast<float>(mapHeight));
    return std::clamp(zoom, kMinZoom, kMaxZoom);
}

Camera fitCamera(int32_t mapWidth, int32_t mapHeight, const SDL_FRect& area) {
    Camera camera;
    camera.zoom = fitZoom(mapWidth, mapHeight, area);
    clampToMap(camera, mapWidth, mapHeight, area);
    return camera;
}

void clampToMap(Camera& camera, int32_t mapWidth, int32_t mapHeight, const SDL_FRect& area) {
    camera.zoom = std::clamp(camera.zoom, fitZoom(mapWidth, mapHeight, area), kMaxZoom);
    camera.x = clampAxis(camera.x, camera.zoom, static_cast<float>(mapWidth), area.x, area.w);
    camera.y = clampAxis(camera.y, camera.zoom, static_cast<float>(mapHeight), area.y, area.h);
}

void pan(Camera& camera, float dx, float dy) {
    camera.x -= dx / camera.zoom;
    camera.y -= dy / camera.zoom;
}

void zoomAt(Camera& camera, SDL_FPoint screen, float factor) {
    const SDL_FPoint anchor = screenToWorld(camera, screen);
    camera.zoom = std::clamp(camera.zoom * factor, kMinZoom, kMaxZoom);
    // Пересчёт от фактического зума: после clamp он может отличаться от запрошенного
    camera.x = anchor.x - screen.x / camera.zoom;
    camera.y = anchor.y - screen.y / camera.zoom;
}

int32_t pickCell(const Camera& camera, SDL_FPoint screen, int32_t mapWidth, int32_t mapHeight) {
    const SDL_FPoint world = screenToWorld(camera, screen);
    // floor, а не усечение: иначе координаты из (-1, 0) попадут в нулевую клетку
    const auto x = static_cast<int32_t>(std::floor(world.x));
    const auto y = static_cast<int32_t>(std::floor(world.y));
    if (!inBounds(x, y, mapWidth, mapHeight)) {
        return -1;
    }
    return idx(x, y, mapWidth);
}

} // namespace genesis
