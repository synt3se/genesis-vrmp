#pragma once

#include <SDL3/SDL.h>

namespace genesis {

// Контекст ImGui и его бэкенды для SDL3 и SDL_Renderer
class ImGuiLayer {
public:
    ImGuiLayer() = default;
    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;
    ~ImGuiLayer();

    bool init(SDL_Window* window, SDL_Renderer* renderer);

    // true, если событие забрал интерфейс и карте его передавать не нужно
    bool handleEvent(const SDL_Event& event);

    void beginFrame();
    void render(SDL_Renderer* renderer);

private:
    bool initialized_ = false;
};

} // namespace genesis
