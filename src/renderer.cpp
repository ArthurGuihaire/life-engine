#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <constants.hpp>
#include <renderer.hpp>
#include <mutex>

// have pixels in static memory
static sf::Color textureBuffer[MAP_HEIGHT][MAP_WIDTH];

// bad design I know, writes to global texture buffer
void generateTextureFromMap(const Map &map) {
  std::lock_guard<std::mutex> lock(map.tiles_mutex);
  const auto tiles = map.tiles;
  for (uint32_t y = 0; y < MAP_HEIGHT; y++) {
    for (uint32_t x = 0; x < MAP_WIDTH; x++) {
      const uint32_t type = static_cast<uint32_t>(tiles[y][x]);
      textureBuffer[y][x] = colorMap[type];
    }
  }
}

Renderer::Renderer(const Map &map, sf::RenderWindow &window)
    : window(window),
      mapTexture(sf::Vector2u{MAP_WIDTH, MAP_HEIGHT}), global_map(map),
      mapSprite(mapTexture) {
  this->window.setVerticalSyncEnabled(true);
  mapSprite.setScale(sf::Vector2f({TILE_SIZE, TILE_SIZE}));
}

void Renderer::render() {
  generateTextureFromMap(this->global_map);

  mapTexture.update(reinterpret_cast<const uint8_t *>(textureBuffer));

  window.draw(this->mapSprite);

  window.display();
}
