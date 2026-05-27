#pragma once
//this file exists because map and creature both need tile
//needed to avoid circular header dependencies

#include <SFML/System/Vector2.hpp>
#include <cstdint>
enum class TileType { NONE, FOOD, GREEN, MOUTH, KILL, MOVE, ARMOR };

struct Tile {
  TileType type;
  sf::Vector2i rel_pos;
};
