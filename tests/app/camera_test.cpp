#include "doctest.h"

#include "genesis/grid.hpp"
#include "render/camera.hpp"

using doctest::Approx;
using genesis::Camera;

TEST_CASE("screenToWorld обратна worldToScreen") {
    const Camera camera{.x = 12.5f, .y = -3.0f, .zoom = 6.0f};
    const SDL_FRect screen = genesis::worldToScreen(camera, SDL_FRect{40.0f, 7.0f, 1.0f, 1.0f});
    const SDL_FPoint world = genesis::screenToWorld(camera, SDL_FPoint{screen.x, screen.y});
    CHECK(world.x == Approx(40.0f));
    CHECK(world.y == Approx(7.0f));
}

TEST_CASE("zoomAt оставляет точку мира под курсором") {
    Camera camera{.x = 10.0f, .y = -5.0f, .zoom = 4.0f};
    const SDL_FPoint cursor{300.0f, 200.0f};
    const SDL_FPoint before = genesis::screenToWorld(camera, cursor);

    genesis::zoomAt(camera, cursor, 2.5f);

    const SDL_FPoint after = genesis::screenToWorld(camera, cursor);
    CHECK(camera.zoom == Approx(10.0f));
    CHECK(after.x == Approx(before.x));
    CHECK(after.y == Approx(before.y));
}

TEST_CASE("zoomAt не выходит за пределы зума") {
    Camera camera{.x = 0.0f, .y = 0.0f, .zoom = 4.0f};
    genesis::zoomAt(camera, SDL_FPoint{0.0f, 0.0f}, 1000.0f);
    CHECK(camera.zoom == genesis::kMaxZoom);
    genesis::zoomAt(camera, SDL_FPoint{0.0f, 0.0f}, 0.0001f);
    CHECK(camera.zoom == genesis::kMinZoom);
}

TEST_CASE("pan: карта едет за мышью ровно на столько же пикселей") {
    Camera camera{.x = 0.0f, .y = 0.0f, .zoom = 8.0f};
    const SDL_FRect before = genesis::worldToScreen(camera, SDL_FRect{5.0f, 5.0f, 1.0f, 1.0f});
    genesis::pan(camera, 24.0f, -16.0f);
    const SDL_FRect after = genesis::worldToScreen(camera, SDL_FRect{5.0f, 5.0f, 1.0f, 1.0f});
    CHECK(after.x - before.x == Approx(24.0f));
    CHECK(after.y - before.y == Approx(-16.0f));
}

TEST_CASE("fitCamera ставит карту по центру и вписывает по меньшей стороне") {
    const Camera camera = genesis::fitCamera(100, 100, SDL_FRect{0.0f, 0.0f, 1600.0f, 900.0f});
    const SDL_FRect map = genesis::worldToScreen(camera, SDL_FRect{0.0f, 0.0f, 100.0f, 100.0f});
    CHECK(map.x + map.w / 2.0f == Approx(800.0f));
    CHECK(map.y + map.h / 2.0f == Approx(450.0f));
    CHECK(map.h == Approx(900.0f));
}

TEST_CASE("fitCamera центрирует карту в области со сдвигом") {
    const SDL_FRect area{400.0f, 50.0f, 1200.0f, 800.0f};
    const Camera camera = genesis::fitCamera(100, 50, area);
    const SDL_FRect map = genesis::worldToScreen(camera, SDL_FRect{0.0f, 0.0f, 100.0f, 50.0f});
    CHECK(map.x + map.w / 2.0f == Approx(area.x + area.w / 2.0f));
    CHECK(map.y + map.h / 2.0f == Approx(area.y + area.h / 2.0f));
    // Карта 2:1 в области 3:2 упирается в ширину
    CHECK(map.w == Approx(area.w));
}

TEST_CASE("pickCell округляет вниз, а не к нулю") {
    const Camera camera{.x = 0.0f, .y = 0.0f, .zoom = 10.0f};
    CHECK(genesis::pickCell(camera, SDL_FPoint{5.0f, 5.0f}, 100, 100) == 0);
    CHECK(genesis::pickCell(camera, SDL_FPoint{15.0f, 25.0f}, 100, 100) == genesis::idx(1, 2, 100));
    // Усечение к нулю дало бы клетку 0
    CHECK(genesis::pickCell(camera, SDL_FPoint{-5.0f, 5.0f}, 100, 100) == -1);
    // Первая клетка за правым краем
    CHECK(genesis::pickCell(camera, SDL_FPoint{1000.0f, 5.0f}, 100, 100) == -1);
}

TEST_CASE("clampToMap не даёт отдалиться дальше всей карты") {
    const SDL_FRect area{300.0f, 0.0f, 1200.0f, 900.0f};
    Camera camera{.x = -50.0f, .y = 20.0f, .zoom = 2.0f};
    genesis::clampToMap(camera, 100, 100, area);
    CHECK(camera.zoom == Approx(genesis::fitZoom(100, 100, area)));
    const SDL_FRect map = genesis::worldToScreen(camera, SDL_FRect{0.0f, 0.0f, 100.0f, 100.0f});
    CHECK(map.x + map.w / 2.0f == Approx(area.x + area.w / 2.0f));
    CHECK(map.y == Approx(area.y));
}

TEST_CASE("clampToMap не пускает край увеличенной карты внутрь области") {
    const SDL_FRect area{300.0f, 0.0f, 1200.0f, 900.0f};
    Camera camera{.x = -40.0f, .y = 500.0f, .zoom = 30.0f};
    genesis::clampToMap(camera, 100, 100, area);
    const SDL_FRect map = genesis::worldToScreen(camera, SDL_FRect{0.0f, 0.0f, 100.0f, 100.0f});
    CHECK(camera.zoom == Approx(30.0f));
    CHECK(map.x == Approx(area.x));
    CHECK(map.y + map.h == Approx(area.y + area.h));
}

TEST_CASE("clampToMap центрирует по оси, где карта меньше области") {
    const SDL_FRect area{0.0f, 0.0f, 1600.0f, 900.0f};
    Camera camera{.x = 7.0f, .y = 30.0f, .zoom = 10.0f};
    genesis::clampToMap(camera, 100, 100, area);
    const SDL_FRect map = genesis::worldToScreen(camera, SDL_FRect{0.0f, 0.0f, 100.0f, 100.0f});
    // По ширине карта (1000) меньше области (1600): по центру. По высоте больше: край на границе
    CHECK(map.x + map.w / 2.0f == Approx(800.0f));
    CHECK(map.y + map.h >= area.y + area.h - 0.001f);
    CHECK(map.y <= area.y + 0.001f);
}
