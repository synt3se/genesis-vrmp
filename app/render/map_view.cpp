#include "render/map_view.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

#include "genesis/grid.hpp"

namespace genesis {

namespace {

constexpr int kBytesPerPixel = 4;
constexpr float kZoomStep = 1.15f;  // множитель зума на одно деление колеса
constexpr SDL_Color kHoverColor{255, 255, 255, 110};
constexpr SDL_Color kSelectedColor{255, 210, 60, 255};

TexturePtr createMapTexture(SDL_Renderer* renderer, int32_t width, int32_t height) {
    // STREAMING: текстура перезаливается при каждой перегенерации
    TexturePtr texture(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
                                         width, height));
    if (!texture) {
        SDL_Log("SDL_CreateTexture: %s", SDL_GetError());
        return nullptr;
    }
    // PIXELART сохраняет клетки резкими и одинаковой ширины при дробном зуме
    if (!SDL_SetTextureScaleMode(texture.get(), SDL_SCALEMODE_PIXELART)) {
        SDL_SetTextureScaleMode(texture.get(), SDL_SCALEMODE_NEAREST);
    }
    return texture;
}

} // namespace

bool MapView::setWorld(SDL_Renderer* renderer, const World& world, const MapStyle& style) {
    const bool resized = !texture_ || world.spec.width != spec_.width || world.spec.height != spec_.height;
    spec_ = world.spec;
    if (resized) {
        texture_ = createMapTexture(renderer, spec_.width, spec_.height);
        if (!texture_) {
            return false;
        }
        selectedCell_ = -1;
        autoFit_ = true;
        fit();
    }

    std::vector<uint8_t> pixels(world.elevation.size() * kBytesPerPixel);
    for (size_t i = 0; i < world.elevation.size(); ++i) {
        const Rgb color = terrainColor(world, static_cast<int32_t>(i), style);
        pixels[i * kBytesPerPixel + 0] = color.r;
        pixels[i * kBytesPerPixel + 1] = color.g;
        pixels[i * kBytesPerPixel + 2] = color.b;
        pixels[i * kBytesPerPixel + 3] = 255;
    }
    SDL_UpdateTexture(texture_.get(), nullptr, pixels.data(), spec_.width * kBytesPerPixel);
    return true;
}

void MapView::handleEvent(SDL_Renderer* renderer, const SDL_Event& windowEvent) {
    // Координаты окна и рендерера расходятся при масштабировании экрана
    SDL_Event event = windowEvent;
    SDL_ConvertEventToRenderCoordinates(renderer, &event);

    switch (event.type) {
    case SDL_EVENT_MOUSE_MOTION: {
        const SDL_FPoint now{event.motion.x, event.motion.y};
        // Разница позиций, а не xrel: позиции уже в координатах рендерера
        if ((event.motion.state & SDL_BUTTON_RMASK) != 0 && mouseInWindow_) {
            pan(camera_, now.x - mouse_.x, now.y - mouse_.y);
            clampToMap(camera_, spec_.width, spec_.height, viewport_);
            autoFit_ = false;
        }
        mouse_ = now;
        mouseInWindow_ = true;
        break;
    }
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        mouseInWindow_ = false;
        break;
    case SDL_EVENT_MOUSE_WHEEL: {
        // Степень, потому что тачпад присылает дробные деления
        float notches = event.wheel.y;
        if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
            notches = -notches;
        }
        zoomAt(camera_, SDL_FPoint{event.wheel.mouse_x, event.wheel.mouse_y}, std::pow(kZoomStep, notches));
        clampToMap(camera_, spec_.width, spec_.height, viewport_);
        autoFit_ = false;
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event.button.button == SDL_BUTTON_LEFT) {
            selectedCell_ = pickCell(camera_, SDL_FPoint{event.button.x, event.button.y}, spec_.width, spec_.height);
        }
        break;
    case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_HOME) {
            autoFit_ = true;
            fit();
        }
        break;
    default:
        break;
    }
}

void MapView::setViewport(const SDL_FRect& area) {
    if (SDL_RectsEqualFloat(&area, &viewport_)) {
        return;
    }
    viewport_ = area;
    if (autoFit_) {
        fit();
    } else {
        clampToMap(camera_, spec_.width, spec_.height, viewport_);
    }
}

void MapView::fit() {
    camera_ = fitCamera(spec_.width, spec_.height, viewport_);
}

void MapView::draw(SDL_Renderer* renderer) const {
    const SDL_FRect mapWorld{0.0f, 0.0f, static_cast<float>(spec_.width), static_cast<float>(spec_.height)};
    const SDL_FRect mapScreen = worldToScreen(camera_, mapWorld);
    SDL_RenderTexture(renderer, texture_.get(), nullptr, &mapScreen);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (mouseInWindow_) {
        drawCellOutline(renderer, pickCell(camera_, mouse_, spec_.width, spec_.height), kHoverColor);
    }
    drawCellOutline(renderer, selectedCell_, kSelectedColor);
}

void MapView::drawCellOutline(SDL_Renderer* renderer, int32_t cell, SDL_Color color) const {
    if (cell < 0) {
        return;
    }
    const SDL_FRect cellWorld{static_cast<float>(cellX(cell, spec_.width)),
                              static_cast<float>(cellY(cell, spec_.width)), 1.0f, 1.0f};
    const SDL_FRect cellScreen = worldToScreen(camera_, cellWorld);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderRect(renderer, &cellScreen);
}

} // namespace genesis
