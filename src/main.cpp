#include <constants.hpp>
#include <map.hpp>
#include <renderer.hpp>
#include <rando.hpp>

int main() {
  random_generator::initialize();
  Tile starting_tiles[3] = {{TileType::MOUTH, {0, 0}},
                            {TileType::GREEN, {-1, -1}},
                            {TileType::GREEN, {1, 1}}};
  Creature starting_creature(starting_tiles, 3,
                             {MAP_WIDTH / 2, MAP_HEIGHT / 2});
  Map map(starting_creature);
  Renderer renderer(map);

  while (renderer.window.isOpen()) {
    map.update();
    renderer.render();
  }
}
