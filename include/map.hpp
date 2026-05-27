#pragma once
#include <SFML/System/Vector2.hpp>
#include <constants.hpp>
#include <tile.hpp>
#include <creature.hpp>
#include <vector>

class Map {
public:
  Map(const Creature &starting_creature);
  void update();
  // public so renderer can make a pixel map
  TileType tiles[MAP_HEIGHT][MAP_WIDTH];

private:
  std::vector<Creature> creatures;

  bool collision(const Creature &creature, const Tile &tile);
};
