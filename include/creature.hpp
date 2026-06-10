#pragma once
#include <boost/container/small_vector.hpp>
#include <tile.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <constants.hpp>

enum class Direction { NONE, RIGHT, UP, LEFT, DOWN };

// hash function for unordered set for finding adjacent tiles
struct Vec2Hash {
  std::size_t operator()(const sf::Vector2i &v) const {
    return static_cast<uint32_t>(v.x) + (static_cast<uint32_t>(v.y) << 16);
  }
};

class Map;

struct Creature {
  Creature(const Tile tiles[], const uint32_t num_tiles,
           const sf::Vector2i starting_position);

  uint32_t num_tiles;
  uint32_t num_adjacent_tiles;
  uint32_t health;
  uint32_t food;
  uint32_t reproduction_cost;
  //creature position relative to top left of the map
  sf::Vector2i position;
  bool can_move; // not const cause of C++ initializer list shenanigans
  Direction movement;
  uint32_t age;

  uint32_t num_food_bonuses;

  Tile tiles[MAX_TILES_PER_CREATURE];
  //positions relative to creature position
  boost::container::small_vector<sf::Vector2i, MAX_TILES_PER_CREATURE> adjacent_tiles;

  bool canRotate(uint32_t num_90_rotations) const;
  Creature reproduce();
  void updateTiles(Map& map);
  void updateMovement(Map& map);
  void prepassKills(Map& map);
  void update(Map& map);
};
