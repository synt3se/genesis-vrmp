#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "genesis/world.hpp"
#include "render/camera.hpp"
#include "render/terrain_color.hpp"
#include "sdl_ptr.hpp"

namespace genesis {

// Карта на экране: текстура мира, камера, наведённая и выбранная клетки.
// Мир не хранит: получает его в setWorld и запоминает только размер
class MapView {
public:
    // Перерисовывает слои карты. При смене размера пересоздаёт текстуру и сбрасывает камеру и выбор
    bool setWorld(SDL_Renderer* renderer, const World& world, const MapStyle& style);

    // Событие в координатах окна, как его присылает SDL
    void handleEvent(SDL_Renderer* renderer, const SDL_Event& event);

    // Курсор ушёл на панель интерфейса
    void clearHover() { mouseInWindow_ = false; }

    // Область окна под карту, в пикселях рендерера. Пока камеру не двигали,
    // карта вписывается в неё заново при каждом изменении
    void setViewport(const SDL_FRect& area);

    void draw(SDL_Renderer* renderer) const;

    int32_t selectedCell() const { return selectedCell_; }

private:
    void fit();
    void drawCellOutline(SDL_Renderer* renderer, int32_t cell, SDL_Color color) const;

    WorldSpec spec_;
    TexturePtr texture_;
    SDL_FRect viewport_{0.0f, 0.0f, 0.0f, 0.0f};
    bool autoFit_ = true;
    Camera camera_;
    SDL_FPoint mouse_{0.0f, 0.0f};  // в координатах рендерера
    bool mouseInWindow_ = false;
    int32_t selectedCell_ = -1;
};

} // namespace genesis
