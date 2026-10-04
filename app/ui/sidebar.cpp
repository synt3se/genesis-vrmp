#include "ui/sidebar.hpp"

#include <cstddef>

#include "imgui.h"

#include "genesis/grid.hpp"

namespace genesis {

namespace {

constexpr int32_t kMinMapSize = 32;
constexpr int32_t kMaxMapSize = 256;
constexpr double kMinWaterFraction = 0.05;
constexpr double kMaxWaterFraction = 0.95;

// Ширина в высотах шрифта, чтобы панель масштабировалась вместе с DPI
constexpr float kSidebarWidthInFonts = 20.0f;
constexpr float kSidebarMinWidthInFonts = 14.0f;
constexpr float kSidebarMaxWidthShare = 0.4f;

} // namespace

bool beginSidebar() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float fontSize = ImGui::GetFontSize();
    const float height = viewport->WorkSize.y;
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(fontSize * kSidebarWidthInFonts, height), ImGuiCond_FirstUseEver);
    // Высота всегда во всё окно, ширину пользователь меняет за правый край
    ImGui::SetNextWindowSizeConstraints(ImVec2(fontSize * kSidebarMinWidthInFonts, height),
                                        ImVec2(viewport->WorkSize.x * kSidebarMaxWidthShare, height));
    return ImGui::Begin("Sidebar", nullptr,
                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
}

float endSidebar() {
    const float width = ImGui::GetWindowWidth() * ImGui::GetIO().DisplayFramebufferScale.x;
    // End вызывается всегда, даже если Begin вернул false
    ImGui::End();
    return width;
}

bool generationSection(uint64_t& seed, WorldSpec& spec) {
    if (!ImGui::CollapsingHeader("Генерация", ImGuiTreeNodeFlags_DefaultOpen)) {
        return false;
    }
    bool changed = false;
    const uint64_t seedStep = 1;
    changed |= ImGui::InputScalar("seed", ImGuiDataType_U64, &seed, &seedStep);
    changed |= ImGui::SliderScalar("ширина", ImGuiDataType_S32, &spec.width, &kMinMapSize, &kMaxMapSize);
    changed |= ImGui::SliderScalar("высота", ImGuiDataType_S32, &spec.height, &kMinMapSize, &kMaxMapSize);
    changed |= ImGui::SliderScalar("доля воды", ImGuiDataType_Double, &spec.waterFraction, &kMinWaterFraction,
                                   &kMaxWaterFraction, "%.2f");
    ImGui::TextDisabled("Пробел: следующий seed");
    return changed;
}

bool viewSection(MapStyle& style, bool& showDemo) {
    if (!ImGui::CollapsingHeader("Вид", ImGuiTreeNodeFlags_DefaultOpen)) {
        return false;
    }
    bool changed = false;
    changed |= ImGui::SliderFloat("рельеф", &style.reliefExaggeration, 0.0f, 80.0f, "%.0f");
    changed |= ImGui::SliderFloat("рассеянный свет", &style.ambient, 0.0f, 1.0f, "%.2f");
    ImGui::Checkbox("демо ImGui", &showDemo);
    ImGui::TextDisabled("ПКМ: сдвиг, колесо: зум, Home: по центру");
    if (showDemo) {
        ImGui::ShowDemoWindow(&showDemo);
    }
    return changed;
}

void inspectorSection(const World& world, int32_t selectedCell) {
    if (!ImGui::CollapsingHeader("Инспектор", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }
    if (selectedCell < 0) {
        ImGui::TextDisabled("Выбери клетку левой кнопкой");
        return;
    }
    const int32_t w = world.spec.width;
    const float elevation = world.elevation[static_cast<size_t>(selectedCell)];
    ImGui::Text("клетка (%d, %d)", cellX(selectedCell, w), cellY(selectedCell, w));
    ImGui::Text("высота %.3f", elevation);
    ImGui::Text("%s", elevation < world.seaLevel ? "вода" : "суша");
}

} // namespace genesis
