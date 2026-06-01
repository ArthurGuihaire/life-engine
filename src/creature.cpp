#include <SFML/System/Vector2.hpp>
#include <creature.hpp>
#include <iostream>
#include <rando.hpp>
#include <cstring>
#include <unordered_set>

Creature::Creature(const Tile tiles[], const uint32_t num_tiles,
                   const sf::Vector2i starting_position)
    : can_move(false), num_tiles(num_tiles),
      reproduction_cost(num_tiles * REPRODUCTION_FOOD_FACTOR), health(1),
      food(0), position(starting_position) {
  std::memcpy(this->tiles, tiles, num_tiles * sizeof(Tile));

  for (uint32_t i = 0; i < num_tiles; i++) {
    if (tiles[i].type == TileType::MOVE)
      this->can_move = true;
    else if (tiles[i].type == TileType::ARMOR)
      this->health += 1;
  }

  // scuffed random
  long rand = random();
  switch (rand & 0x3) {
  case 0:
    movement = Direction::UP;
  case 1:
    movement = Direction::RIGHT;
  case 2:
    movement = Direction::DOWN;
  case 3:
    movement = Direction::LEFT;
  }
}

// hash function for unordered set for finding adjacent tiles
struct Vec2Hash {
  std::size_t operator()(const sf::Vector2i &v) const {
    return static_cast<uint32_t>(v.x) + (static_cast<uint32_t>(v.y) << 16);
  }
};

// this is inefficeint, copies the tiles twice but can technically be reduced to once
// does not randomize position or direction, that is handled by randomizePlacement
Creature Creature::reproduce() {
  Tile buffer[MAX_TILES_PER_CREATURE];
  std::memcpy(buffer, this->tiles, this->num_tiles * sizeof(Tile));
  uint32_t new_num_tiles = this->num_tiles;

  float mutation = rando();
  if (mutation < ADD_TILE_MUTATION_CHANCE &&
      num_tiles != MAX_TILES_PER_CREATURE) {
    // BROWNIE MAGIC
    std::unordered_set<sf::Vector2i, Vec2Hash> empty_adj;
    sf::Vector2i empty_adj_array[MAX_TILES_PER_CREATURE * 4];
    uint32_t num_empty_adj = 0;
    for (uint32_t i = 0; i < num_tiles; i++) {
      sf::Vector2i adj_array[4] = {
          buffer[i].rel_pos + adjacent_offsets[0],
          buffer[i].rel_pos + adjacent_offsets[1],
          buffer[i].rel_pos + adjacent_offsets[2],
          buffer[i].rel_pos + adjacent_offsets[3],
      };
      for (sf::Vector2i adj : adj_array) {
        if (empty_adj.contains(adj)) {
          empty_adj.insert(adj);
          empty_adj_array[num_empty_adj++] = adj;
        }
      }
    }
    TileType type = static_cast<TileType>((uint32_t)(rando() * NUM_TILE_TYPES));

    const uint32_t num = (uint32_t)(rando() * num_empty_adj);
    const Tile new_tile = Tile{type, empty_adj_array[num]};
    buffer[new_num_tiles++] = new_tile;
  } else if (mutation < REMOVE_TILE_MUTATION_CHANCE && num_tiles > 1) {
    const uint32_t num = (uint32_t)(rando() * num_tiles);
    std::cout << "removed tile\n";
    // to delete, overwrite with the last element
    buffer[num] = buffer[--new_num_tiles];
  } else if (mutation < CHANGE_TILE_MUTATION_CHANCE) {
    const uint32_t num = (uint32_t)(rando() * num_tiles);
    const TileType new_type = (TileType)(rando() * NUM_TILE_TYPES);
    buffer[num].type = new_type;
  }

  return Creature(buffer, new_num_tiles, this->position);
}

void Creature::randomizePlacement(sf::Vector2i center) {

}
