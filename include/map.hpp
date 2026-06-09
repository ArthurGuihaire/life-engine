#pragma once
#include <creature.hpp>
#include <SFML/System/Vector2.hpp>
#include <chrono>
#include <constants.hpp>
#include <sys/types.h>
#include <tile.hpp>
#include <vector>

class Map {
public:
  Map(const Creature &starting_creature);
  void update();
  // uint8_t getAdjacentTiles(const sf::Vector2i& pos, TileType* adjacent_tiles[4]);
  void erase(const Creature& creature, const TileType type);
  void kill(const uint32_t idx);
  void writeToMap(const Creature& creature);
  bool collision(const sf::Vector2i& pos);
  uint32_t numCreatures() const { return creatures.size(); }
  uint32_t cascadeEat(const sf::Vector2i& position);

  // public so renderer can make a pixel map
  TileType tiles[MAP_HEIGHT][MAP_WIDTH];

private:
  std::vector<Creature> creatures;
  std::chrono::time_point<std::chrono::steady_clock> next_update_time;
};
