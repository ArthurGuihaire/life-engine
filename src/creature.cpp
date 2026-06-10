#include <boost/container/container_fwd.hpp>
#include <constants.hpp>
#include <iostream>
#include <tile.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <creature.hpp>
#include <map.hpp>
#include <cstdint>
#include <utils.hpp>
#include <cstring>

Creature::Creature(const Tile tiles[], const uint32_t num_tiles,
                   const sf::Vector2i starting_position)
    : can_move(false), num_tiles(num_tiles),
      reproduction_cost(num_tiles * REPRODUCTION_FOOD_FACTOR + REPRODUCTION_FOOD_BASE), health(1),
      food(0), position(starting_position), age(0), num_adjacent_tiles(0), num_food_bonuses(0) {
  std::memcpy(this->tiles, tiles, num_tiles * sizeof(Tile));

  for (uint32_t i = 0; i < num_tiles; i++) {
    if (tiles[i].type == TileType::MOVE)
      this->can_move = true;
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

  //O(n^2) but n is small and no heap allocations
  for (uint32_t i = 0; i < this->num_tiles; i++) {
    const sf::Vector2i& tile_pos = this->tiles[i].rel_pos;
    for (const sf::Vector2i& offset : adjacent_offsets) {
      const sf::Vector2i adj_pos = tile_pos + offset;
      for (uint32_t j = 0; j < this->num_tiles; j++) {
        if (this->tiles[j].rel_pos == adj_pos) {
          goto after_both_loops;
        }
      }
      bool exists = false;
      for (uint32_t j = 0; j < this->num_adjacent_tiles; j++) {
        if (this->adjacent_tiles[j] == adj_pos) {
          exists = true;
          break;
        }
      }
      if (!exists) {
        this->adjacent_tiles.push_back(adj_pos);
        this->num_adjacent_tiles++;
      }
      if (this->tiles[i].type == TileType::KILL) {
        exists = false;
        for (uint32_t j = 0; j < this->adjacent_red_tiles.size(); j++) {
          if (this->adjacent_red_tiles[j] == adj_pos) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          this->adjacent_red_tiles.push_back(adj_pos);
        }
      }
      else if (this->tiles[i].type == TileType::MOUTH) {
        exists = false;
        for (uint32_t j = 0; j < this->adjacent_yellow_tiles.size(); j++) {
          if (this->adjacent_yellow_tiles[j] == adj_pos) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          this->adjacent_yellow_tiles.push_back(adj_pos);
        }
      }
      if (this->tiles[i].type != TileType::ARMOR) {
        exists = false;
        for (uint32_t j = 0; j < this->adjacent_vulnerable_tiles.size(); j++) {
          if (this->adjacent_vulnerable_tiles[j] == adj_pos) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          this->adjacent_vulnerable_tiles.push_back(adj_pos);
        }
      }
    }
    after_both_loops:;
  }
}

// this is inefficeint, copies the tiles twice but can technically be reduced to once
// does not randomize position or direction, that is handled by randomizePlacement
Creature Creature::reproduce() {
  Tile buffer[MAX_TILES_PER_CREATURE];

  uint32_t new_num_tiles = this->num_tiles;

  std::memcpy(buffer, this->tiles, this->num_tiles * sizeof(Tile));

  const double mutation = rando();
  if (mutation < ADD_TILE_MUTATION_CHANCE) {
    if (num_tiles < MAX_TILES_PER_CREATURE && this->num_adjacent_tiles > 0) {
      TileType type = static_cast<TileType>((uint32_t)(rando() * (NUM_TILE_TYPES - NUM_NONLIVING_TYPES)) + NUM_NONLIVING_TYPES);
      const uint32_t num = (uint32_t)(rando() * this->num_adjacent_tiles);
      const Tile new_tile = Tile{type, this->adjacent_tiles[num]};
      buffer[new_num_tiles++] = new_tile;
    }
  } else if (mutation < REMOVE_TILE_MUTATION_CHANCE && new_num_tiles > 1) {
    const uint32_t num = (uint32_t)(rando() * new_num_tiles);
    // to delete, overwrite with the last element
    buffer[num] = buffer[--new_num_tiles];
  } else if (mutation < CHANGE_TILE_MUTATION_CHANCE) {
    const uint32_t num = (uint32_t)(rando() * new_num_tiles);
    const TileType new_type = (TileType)(rando() * (NUM_TILE_TYPES - NUM_NONLIVING_TYPES) + NUM_NONLIVING_TYPES);
    buffer[num].type = new_type;
  }

  this->food -= this->reproduction_cost;

  const uint32_t num_rotations = (uint32_t)(rando() * 4);
  // std::memcpy(buffer, this->tiles, this->num_tiles * sizeof(Tile));
  for (uint32_t i = 0; i < new_num_tiles; ++i) {
    buffer[i].rel_pos = rotate_vector(buffer[i].rel_pos, num_rotations);
  }

  return Creature(buffer, new_num_tiles, this->position);
}

void Creature::updateTiles(Map& map) {
  // update green tiles
  for (uint32_t idx = 0; idx < this->num_tiles; ++idx) {
    const Tile &tile = this->tiles[idx];
    const sf::Vector2i base_pos = this->position + tile.rel_pos;

    if ((!this->can_move) && tile.type == TileType::GREEN) {
      for (const sf::Vector2i& offset : adjacent_offsets) {
        const sf::Vector2i adj_pos = base_pos + offset;
        if (!isOnMap(adj_pos)) {
          continue;
        }
        TileType& adjacent_tile = map.tiles[adj_pos.y][adj_pos.x];
        if (adjacent_tile == TileType::NONE && rando() < GREEN_PRODUCTION_CHANCE) {
          std::lock_guard<std::mutex> lock(map.tiles_mutex);
          adjacent_tile = TileType::FOOD;
        }
      }
    }
  }

  //update mouth tiles
  for (const sf::Vector2i& adj_pos : this->adjacent_yellow_tiles) {
    const sf::Vector2i abs_pos = adj_pos + this->position;
    if (!isOnMap(abs_pos)) continue;
    if (map.tiles[abs_pos.y][abs_pos.x] == TileType::FOOD) {
      if (CASCADE_EAT) {
        this->food += map.cascadeEat(abs_pos);
      }
      else {
        this->food++;
        std::lock_guard<std::mutex> lock(map.tiles_mutex);
        map.tiles[abs_pos.y][abs_pos.x] = TileType::NONE;
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
      const uint32_t num_90_rotations = (uint32_t)(rando() * 4);
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

          for (uint32_t i = 0; i < this->num_adjacent_tiles; i++) {
            this->adjacent_tiles[i] = rotate_vector(this->adjacent_tiles[i], num_90_rotations);
          }
          for (uint32_t i = 0; i < this->adjacent_red_tiles.size(); i++) {
            this->adjacent_red_tiles[i] = rotate_vector(this->adjacent_red_tiles[i], num_90_rotations);
          }
          for (uint32_t i = 0; i < this->adjacent_yellow_tiles.size(); i++) {
            this->adjacent_yellow_tiles[i] = rotate_vector(this->adjacent_yellow_tiles[i], num_90_rotations);
          }
          for (uint32_t i = 0; i < this->adjacent_vulnerable_tiles.size(); i++) {
            this->adjacent_vulnerable_tiles[i] = rotate_vector(this->adjacent_vulnerable_tiles[i], num_90_rotations);
          }
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

void Creature::updateHealth(const Map& map) {
  for (const sf::Vector2i& adj_pos : this->adjacent_vulnerable_tiles) {
    const sf::Vector2i abs_pos = this->position + adj_pos;
    if (!isOnMap(abs_pos)) continue;
    if (map.tiles[abs_pos.y][abs_pos.x] == TileType::KILL) {
      this->health -= 1;
    }
  }
}

void Creature::updateKillBonus(const Map& map) {
  for (const sf::Vector2i& adj_pos : this->adjacent_red_tiles) {
    const sf::Vector2i abs_pos = adj_pos + this->position;
    if (!isOnMap(abs_pos)) continue;
    const TileType& type = map.tiles[abs_pos.y][abs_pos.x];
    if (type != TileType::NONE && type != TileType::FOOD && type != TileType::ARMOR) {
      this->food += KILL_FOOD_BONUS;
    }
  }
}

bool Creature::shouldDie(const Map& map) const {
  const uint32_t MAX_AGE = MAX_AGE_FACTOR * this->num_tiles;
  return this->health <= 0 || this->age >= MAX_AGE;
}

bool Creature::hasDuplicateTiles() const {
  for (uint32_t i = 0; i < this->num_tiles; i++) {
    const sf::Vector2i& tile_pos = this->tiles[i].rel_pos;
    for (uint32_t j = 0; j < i; j++) {
      if (tile_pos == this->tiles[j].rel_pos) {
        return true;
      }
    }
  }
  return false;
}

void Creature::update(Map& map) {
  this->updateTiles(map);
  this->updateMovement(map);
  this->age++;
  if (this->hasDuplicateTiles()) {
    std::cout << "Duplicate tiles detected" << std::endl;

  }
}
