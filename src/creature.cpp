#include <constants.hpp>
#include <iostream>
#include <iterator>
#include <tile.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <creature.hpp>
#include <map.hpp>
#include <cstdint>
#include <utils.hpp>
#include <cstring>
#include <unordered_set>

Creature::Creature(const Tile tiles[], const uint32_t num_tiles,
                   const sf::Vector2i starting_position)
    : can_move(false), num_tiles(num_tiles),
      reproduction_cost(num_tiles * REPRODUCTION_FOOD_FACTOR + REPRODUCTION_FOOD_BASE), health(1),
      food(0), position(starting_position), age(0) {
  std::memcpy(this->tiles, tiles, num_tiles * sizeof(Tile));

  for (uint32_t i = 0; i < num_tiles; i++) {
    if (tiles[i].type == TileType::MOVE) {
      this->can_move = true;
    }
    else if (tiles[i].type == TileType::ARMOR)
      this->health += 1;
  }

  if (this->can_move) {
    // scuffed random
    uint32_t rand = (uint32_t)(rando() * 4);
    switch (rand) {
    case 0:
      movement = Direction::UP;
      break;
    case 1:
      movement = Direction::RIGHT;
      break;
    case 2:
      movement = Direction::DOWN;
      break;
    case 3:
      movement = Direction::LEFT;
      break;
    }
  }
  else {
    movement = Direction::NONE;
  }

  /*
  //O(n^2) but n is small and no heap allocations
  for (uint32_t i = 0; i < this->num_tiles; i++) {
    sf::Vector2i tile_pos = this->position + this->tiles[i].rel_pos;
    for (sf::Vector2i offset : adjacent_offsets) {
      sf::Vector2i adj_pos = tile_pos + offset;
      for (uint32_t j = 0; j < i; j++) {
        if (this->tiles[j].rel_pos == adj_pos - this->position) {
          this->adjacent_tiles[this->num_adjacent_tiles] = adj_pos - this->position;
          this->num_adjacent_tiles++;
        }
      }
    }
  }
  */
}

// this is inefficeint, copies the tiles twice but can technically be reduced to once
// does not randomize position or direction, that is handled by randomizePlacement
Creature Creature::reproduce() {
  Tile buffer[MAX_TILES_PER_CREATURE];

  const uint32_t num_rotations = (uint32_t)(rando() * 4);
  // std::memcpy(buffer, this->tiles, this->num_tiles * sizeof(Tile));
  for (uint32_t i = 0; i < this->num_tiles; ++i) {
    buffer[i].type = this->tiles[i].type;
    buffer[i].rel_pos = rotate_vector(this->tiles[i].rel_pos, num_rotations);
  }
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
        bool occupied = false;
        for (uint32_t j = 0; j < new_num_tiles; j++) {
          if (buffer[j].rel_pos == adj) {
            occupied = true;
            break;
          }
        }
        if (!occupied && !empty_adj.contains(adj)) {
          empty_adj.insert(adj);
          empty_adj_array[num_empty_adj++] = adj;
        }
      }
    }
    if (num_empty_adj > 0) {
      TileType type = static_cast<TileType>((uint32_t)(rando() * (NUM_TILE_TYPES - NUM_NONLIVING_TYPES)) + NUM_NONLIVING_TYPES);

      const uint32_t num = (uint32_t)(rando() * num_empty_adj);
      const Tile new_tile = Tile{type, empty_adj_array[num]};
      buffer[new_num_tiles++] = new_tile;
    }
  } else if (mutation < REMOVE_TILE_MUTATION_CHANCE && num_tiles > 1) {
    const uint32_t num = (uint32_t)(rando() * num_tiles);
    // to delete, overwrite with the last element
    buffer[num] = buffer[--new_num_tiles];
  } else if (mutation < CHANGE_TILE_MUTATION_CHANCE) {
    const uint32_t num = (uint32_t)(rando() * num_tiles);
    const TileType new_type = (TileType)(rando() * (NUM_TILE_TYPES - NUM_NONLIVING_TYPES) + NUM_NONLIVING_TYPES);
    buffer[num].type = new_type;
  }

  this->food -= this->reproduction_cost;

  return Creature(buffer, new_num_tiles, this->position);
}

void Creature::updateTiles(Map& map) {
  for (uint32_t idx = 0; idx < this->num_tiles; ++idx) {
    const Tile &tile = this->tiles[idx];
    const sf::Vector2i base_pos = this->position + tile.rel_pos;
    // very safe, a C-style array of pointers
    TileType* adjacent_tiles[4];
    const uint8_t mask = map.getAdjacentTiles(base_pos, adjacent_tiles);

    // I know I know, im checking can_move for every tile, too lazy to split
    // into 2 for loops
    if ((!this->can_move) && tile.type == TileType::GREEN) {
      for (uint32_t i = 0; i < sizeof(adjacent_tiles) / sizeof(adjacent_tiles[0]); i++) {
        if (mask & (1 << i) && *adjacent_tiles[i] == TileType::NONE && rando() < GREEN_PRODUCTION_CHANCE) {
          *adjacent_tiles[i] = TileType::FOOD;
        }
      }
    }
    if (tile.type == TileType::MOUTH) {
      for (uint32_t i = 0; i < sizeof(adjacent_tiles) / sizeof(adjacent_tiles[0]); i++) {
        if (mask & (1 << i) && *adjacent_tiles[i] == TileType::FOOD) {
          this->food += 1;
          *adjacent_tiles[i] = TileType::NONE;
        }
      }
    }
  }
}


void Creature::updateMovement(Map& map) {
  // handle movement
  if (this->can_move) {
    bool blocked = false;
    const sf::Vector2i rel_vec = get_direction_vector(this->movement);
    for (uint32_t idx = 0; idx < this->num_tiles; ++idx) {
      const Tile &tile = this->tiles[idx];
      const sf::Vector2i abs_vec = rel_vec + this->position + tile.rel_pos;
      if (map.collision(abs_vec)) {
        blocked = true;
        break;
      }
    }

    if (blocked || rando() < ROTATION_CHANCE) {
      const Direction new_dir = (Direction)(rando() * 4 + 1);
      const uint32_t num_90_rotations = (uint32_t)new_dir - ((uint32_t)this->movement + 4) % 4;
      if (num_90_rotations != 0) {
        bool rotation_blocked = false;
        Tile new_arr[MAX_TILES_PER_CREATURE];
        for (uint32_t i = 0; i < this->num_tiles; i++) {
          Tile &tile = this->tiles[i];
          const sf::Vector2i new_vec = rotate_vector(tile.rel_pos, num_90_rotations);
          const sf::Vector2i abs_pos = this->position + new_vec;
          if (map.collision(abs_pos)) {
            rotation_blocked = true;
            break;
          } else
            new_arr[i] = {tile.type, new_vec};
        }

        if (!rotation_blocked) {
          map.erase(*this, TileType::NONE);
          std::memcpy(this->tiles, new_arr,
                      this->num_tiles * sizeof(Tile));
          map.writeToMap(*this);
        }
        this->movement = new_dir;
        this->updateMovement(map);
      }
    }

    else {
      map.erase(*this, TileType::NONE);
      this->position += rel_vec;
      map.writeToMap(*this);
    }
  }
}

void Creature::prepassKills(Map& map) {
  for (uint32_t idx = 0; idx < this->num_tiles; ++idx) {
    const Tile &tile = this->tiles[idx];
    if (tile.type == TileType::KILL) {
      for (sf::Vector2i adj : adjacent_offsets) {
        const sf::Vector2i abs_vec = this->position + tile.rel_pos + adj;
        if (map.collision(abs_vec)) {
          this->food += KILL_FOOD_BONUS;
          break;
        }
      }
    }
  }
}

void Creature::update(Map& map) {
  this->prepassKills(map);
  this->updateTiles(map);
  this->updateMovement(map);
}
