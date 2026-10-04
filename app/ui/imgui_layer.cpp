#include "ui/imgui_layer.hpp"

#include <array>
#include <filesystem>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

namespace genesis {

namespace {

constexpr float kFontSize = 16.0f;

// Встроенный шрифт ImGui без кириллицы, поэтому берём системный.
// Если ни одного нет, подписи на русском превратятся в знаки вопроса, но приложение работает
constexpr std::array kFontCandidates{
    "C:/Windows/Fonts/segoeui.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
};

void loadFont() {
    ImGuiIO& io = ImGui::GetIO();
    for (const char* path : kFontCandidates) {
        std::error_code error;
        if (std::filesystem::exists(path, error) && io.Fonts->AddFontFromFileTTF(path, kFontSize) != nullptr) {
            return;
        }
    }
    SDL_Log("no font with Cyrillic found, using the built-in one");
}

bool isMouseEvent(Uint32 type) {
    return type == SDL_EVENT_MOUSE_MOTION || type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
           type == SDL_EVENT_MOUSE_BUTTON_UP || type == SDL_EVENT_MOUSE_WHEEL;
}

bool isKeyboardEvent(Uint32 type) {
    return type == SDL_EVENT_KEY_DOWN || type == SDL_EVENT_KEY_UP || type == SDL_EVENT_TEXT_INPUT;
}

} // namespace

ImGuiLayer::~ImGuiLayer() {
    if (initialized_) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }
}

bool ImGuiLayer::init(SDL_Window* window, SDL_Renderer* renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    const float scale = SDL_GetDisplayContentScale(SDL_GetDisplayForWindow(window));
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;
    loadFont();

    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer)) {
        ImGui::DestroyContext();
        return false;
    }
    if (!ImGui_ImplSDLRenderer3_Init(renderer)) {
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        return false;
    }
    initialized_ = true;
    return true;
}

bool ImGuiLayer::handleEvent(const SDL_Event& event) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    const ImGuiIO& io = ImGui::GetIO();
    return (isMouseEvent(event.type) && io.WantCaptureMouse) ||
           (isKeyboardEvent(event.type) && io.WantCaptureKeyboard);
}

void ImGuiLayer::beginFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render(SDL_Renderer* renderer) {
    ImGui::Render();
    // ImGui рисует в координатах окна. Масштаб возвращается к 1, потому что карта
    // и перевод событий мыши работают в пикселях рендерера
    const ImVec2 scale = ImGui::GetIO().DisplayFramebufferScale;
    SDL_SetRenderScale(renderer, scale.x, scale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

} // namespace genesis
