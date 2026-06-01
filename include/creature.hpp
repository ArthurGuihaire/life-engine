#pragma once
#include <SFML/System/Vector2.hpp>
#include <tile.hpp>
#include <constants.hpp>

enum class Direction { NONE, RIGHT, UP, LEFT, DOWN };

struct Creature {
  Creature(const Tile tiles[], const uint32_t num_tiles,
           const sf::Vector2i starting_position);
  Tile tiles[MAX_TILES_PER_CREATURE];
  uint32_t num_tiles; // cause c++ is fun
  uint32_t health;
  uint32_t food;
  uint32_t reproduction_cost;
  sf::Vector2i position;
  bool can_move; // not const cause of C++ initializer list shenanigans
  Direction movement;

  bool canRotate(uint32_t num_90_rotations);
  void randomizePlacement(sf::Vector2i center);
  Creature reproduce();
};
