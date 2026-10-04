#pragma once

#include <cstdint>

#include "genesis/world.hpp"

namespace genesis {

// Заглушка генератора для отладки отрисовки. Вне core/ требования детерминизма не действуют
World makeTestWorld(const WorldSpec& spec, uint64_t seed);

} // namespace genesis
