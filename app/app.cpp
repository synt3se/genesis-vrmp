#include "app.hpp"

#include "test_world.hpp"
#include "ui/sidebar.hpp"

namespace genesis {

namespace {

constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 800;
constexpr SDL_Color kBackground{20, 24, 28, SDL_ALPHA_OPAQUE};

} // namespace

bool App::init(uint64_t seed) {
    // Окно задаётся в логических единицах, поэтому размер домножается на масштаб экрана
    const float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("Genesis", static_cast<int>(kWindowWidth * scale),
                                     static_cast<int>(kWindowHeight * scale),
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY, &window, &renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer: %s", SDL_GetError());
        return false;
    }
    window_.reset(window);
    renderer_.reset(renderer);
    SDL_SetRenderVSync(renderer, 1);

    if (!imgui_.init(window, renderer)) {
        SDL_Log("ImGui backend init failed");
        return false;
    }
    seed_ = seed;
    return regenerate();
}

SDL_AppResult App::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    if (imgui_.handleEvent(event)) {
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            mapView_.clearHover();
        }
        return SDL_APP_CONTINUE;
    }
    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
        if (event.key.key == SDLK_SPACE) {
            ++seed_;
            return regenerate() ? SDL_APP_CONTINUE : SDL_APP_FAILURE;
        }
    }
    mapView_.handleEvent(renderer_.get(), event);
    return SDL_APP_CONTINUE;
}

void App::iterate() {
    imgui_.beginFrame();
    drawUi();

    SDL_Renderer* renderer = renderer_.get();
    SDL_SetRenderDrawColor(renderer, kBackground.r, kBackground.g, kBackground.b, kBackground.a);
    SDL_RenderClear(renderer);
    mapView_.draw(renderer);
    imgui_.render(renderer);
    SDL_RenderPresent(renderer);
}

bool App::regenerate() {
    world_ = makeTestWorld(spec_, seed_);
    return mapView_.setWorld(renderer_.get(), world_, style_);
}

void App::drawUi() {
    if (beginSidebar()) {
        if (generationSection(seed_, spec_)) {
            regenerate();
        }
        if (viewSection(style_, showDemo_)) {
            mapView_.setWorld(renderer_.get(), world_, style_);
        }
        inspectorSection(world_, mapView_.selectedCell());
    }
    const float sidebarWidth = endSidebar();

    int outputWidth = 0;
    int outputHeight = 0;
    SDL_GetRenderOutputSize(renderer_.get(), &outputWidth, &outputHeight);
    mapView_.setViewport(SDL_FRect{sidebarWidth, 0.0f, static_cast<float>(outputWidth) - sidebarWidth,
                                   static_cast<float>(outputHeight)});
}

} // namespace genesis
