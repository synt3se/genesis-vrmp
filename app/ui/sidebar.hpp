#pragma once

#include <cstdint>

#include "genesis/world.hpp"
#include "render/terrain_color.hpp"

namespace genesis {

// Панель слева на всю высоту окна. Секции рисуются между begin и end, только если begin вернул true.
// endSidebar вызывается всегда и возвращает ширину панели в пикселях рендерера
bool beginSidebar();
float endSidebar();

// Секции возвращают true, если пользователь изменил данные
bool generationSection(uint64_t& seed, WorldSpec& spec);
bool viewSection(MapStyle& style, bool& showDemo);
void inspectorSection(const World& world, int32_t selectedCell);

} // namespace genesis
