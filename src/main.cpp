#include <SFML/System/Vector2.hpp>
#include <constants.hpp>
#include <map.hpp>
#include <optional>
#include <renderer.hpp>
#include <utils.hpp>
#include <iostream>

struct ProfilePoint {
  uint32_t num_creatures;
  double update_time;
};

int main() {
  random_generator::initialize();
  // Tile starting_tiles[3] = {{TileType::GREEN, {0, 0}},
  //                           {TileType::MOUTH, {-1, -1}},
  //                           {TileType::MOUTH, {1, 1}}};
  Tile starting_tiles[3] = {{TileType::GREEN, {0, 0}},
                            {TileType::MOUTH, {-1, -1}},
                            {TileType::MOUTH, {1, 1}}};
  Creature starting_creature(starting_tiles, 3,
                             {MAP_WIDTH / 2, MAP_HEIGHT / 2});
  Map map(starting_creature);
  Renderer renderer(map);

  std::vector<ProfilePoint> profiling_data;

  while (renderer.window.isOpen()) {
    if (RUN_PROFILING) {
      auto start_time = std::chrono::steady_clock::now();
      map.update();
      auto end_time = std::chrono::steady_clock::now();
      profiling_data.push_back({map.numCreatures(), std::chrono::duration<double>(end_time - start_time).count()});
    }
    else {
      map.update();
    }
    renderer.render();
    while (const std::optional event = renderer.window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        renderer.window.close();
      }
      else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
      {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
          renderer.window.close();
      }
    }
  }

  if (RUN_PROFILING) {
    for (const auto& point : profiling_data) {
      std::cout << "(" << point.num_creatures << ", " << point.update_time << ")\n";
    }
  }
}
