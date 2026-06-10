#pragma once
#include <creature.hpp>
#include <constants.hpp>
#include <tile.hpp>
#include <SFML/System/Vector2.hpp>
#include <mutex>
#include <vector>
#include <boost/container/small_vector.hpp>

constexpr inline bool isOnMap(const sf::Vector2i& pos) { return pos.x >= 0 && pos.y >= 0 && pos.x < MAP_WIDTH && pos.y < MAP_HEIGHT; }

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
  mutable std::mutex tiles_mutex; //anything can lock the map

private:
  std::vector<Creature> creatures;
  std::vector<Creature> new_creatures;
  boost::container::small_vector<uint32_t, KILL_CREATURES_BUFFER> indices_to_kill;
};
