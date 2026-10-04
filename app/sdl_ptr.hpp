#pragma once

#include <SDL3/SDL.h>

#include <memory>

namespace genesis {

struct SdlDeleter {
    void operator()(SDL_Window* window) const { SDL_DestroyWindow(window); }
    void operator()(SDL_Renderer* renderer) const { SDL_DestroyRenderer(renderer); }
    void operator()(SDL_Texture* texture) const { SDL_DestroyTexture(texture); }
};

using WindowPtr = std::unique_ptr<SDL_Window, SdlDeleter>;
using RendererPtr = std::unique_ptr<SDL_Renderer, SdlDeleter>;
using TexturePtr = std::unique_ptr<SDL_Texture, SdlDeleter>;

} // namespace genesis
