#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <charconv>
#include <cstdint>
#include <memory>
#include <string_view>
#include <system_error>

#include "app.hpp"

namespace {

constexpr uint64_t kDefaultSeed = 42;

uint64_t seedFromArgs(int argc, char* argv[]) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string_view(argv[i]) != "--seed") {
            continue;
        }
        const std::string_view value(argv[i + 1]);
        uint64_t seed = 0;
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), seed);
        if (error == std::errc() && end == value.data() + value.size()) {
            return seed;
        }
        SDL_Log("invalid --seed: %s", argv[i + 1]);
    }
    return kDefaultSeed;
}

genesis::App& appFrom(void* appstate) {
    return *static_cast<genesis::App*>(appstate);
}

} // namespace

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    auto app = std::make_unique<genesis::App>();
    if (!app->init(seedFromArgs(argc, argv))) {
        return SDL_APP_FAILURE;
    }
    // Владение переходит к SDL и возвращается в SDL_AppQuit
    *appstate = app.release();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    return appFrom(appstate).handleEvent(*event);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    appFrom(appstate).iterate();
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/) {
    // nullptr, если SDL_AppInit завершился ошибкой
    const std::unique_ptr<genesis::App> app(static_cast<genesis::App*>(appstate));
}
