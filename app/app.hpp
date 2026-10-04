#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

#include "genesis/world.hpp"
#include "render/map_view.hpp"
#include "render/terrain_color.hpp"
#include "sdl_ptr.hpp"
#include "ui/imgui_layer.hpp"

namespace genesis {

class App {
public:
    bool init(uint64_t seed);
    SDL_AppResult handleEvent(const SDL_Event& event);
    void iterate();

private:
    bool regenerate();
    void drawUi();

    // Поля уничтожаются в обратном порядке: карта и ImGui раньше рендерера, рендерер раньше окна
    WindowPtr window_;
    RendererPtr renderer_;
    ImGuiLayer imgui_;

    // Параметры правит интерфейс; мир пересоздаётся из них в regenerate
    uint64_t seed_ = 0;
    WorldSpec spec_;
    MapStyle style_;
    bool showDemo_ = false;

    World world_;
    MapView mapView_;
};

} // namespace genesis
