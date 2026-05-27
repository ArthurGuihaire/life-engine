#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <constants.hpp>
#include <iostream>
#include <renderer.hpp>
// sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML Window");
// window.setVerticalSyncEnabled(true);

// sf::RectangleShape rectangle({120.0f, 50.0f});
// rectangle.setFillColor(sf::Color::Blue);
// while (window.isOpen()) {
//     window.clear(sf::Color::Black);
//     window.draw(rectangle);
//     std::cout << "frame";
//     window.display();
// }

// have pixels in static memory
static sf::Color textureBuffer[MAP_HEIGHT][MAP_WIDTH];

// bad design I know, writes to global texture buffer
void generateTextureFromMap(const Map &map) {
  const auto tiles = map.tiles;
  for (uint32_t y = 0; y < MAP_HEIGHT; y++) {
    for (uint32_t x = 0; x < MAP_WIDTH; x++) {
      const uint32_t type = static_cast<uint32_t>(tiles[y][x]);
      textureBuffer[y][x] = colorMap[type];

      if (type != 0) {
        std::cout << static_cast<uint32_t>(colorMap[type].r) << ", "
                  << static_cast<uint32_t>(colorMap[type].g) << ", "
                  << static_cast<uint32_t>(colorMap[type].b) << std::endl;
      }
    }
  }
}

Renderer::Renderer(const Map &map)
    : window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "SFML Window"),
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
