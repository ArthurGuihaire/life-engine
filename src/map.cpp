#include "tile.hpp"
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <constants.hpp>
#include <iostream>
#include <iterator>
#include <rando.hpp>
#include <cstring>
#include <map.hpp>
#include <thread>

Map::Map(const Creature &starting_creature) : tiles{TileType::NONE} {
  creatures.push_back(starting_creature);

  for (uint32_t i = 0; i < starting_creature.num_tiles; i++) {
    const Tile &tile = starting_creature.tiles[i];
    const sf::Vector2i pos = starting_creature.position + tile.rel_pos;
    tiles[pos.y][pos.x] = tile.type;
  }
}

sf::Vector2i rotate_vector(const sf::Vector2i input,
                           const uint32_t num_rotations) {
  switch (num_rotations) {
  case 0:
    return input;
  case 1:
    return {-input.y, input.x};
  case 2:
    return -input;
  default:
    return {input.y, -input.x};
  }
}

sf::Vector2i get_direction_vector(const Direction direction) {
  switch (direction) {
  case Direction::RIGHT:
    return {1, 0};
  case Direction::UP:
    return {0, 1};
  case Direction::LEFT:
    return {-1, 0};
  case Direction::DOWN:
    return {0, -1};
  default:
    return {0, 0};
  }
}

void Map::update() {
  std::vector<Creature> new_creatures;
  new_creatures.reserve(this->creatures.size());
  // this is really bad
  for (Creature &creature : this->creatures) {
    for (const Tile &tile : creature.tiles) {
      const sf::Vector2i base_pos = creature.position + tile.rel_pos;
      // very safe, a C-style array of pointers
      TileType *adjacent_tiles[4] = {&tiles[base_pos.y][base_pos.x + 1],
                                     &tiles[base_pos.y][base_pos.x - 1],
                                     &tiles[base_pos.y + 1][base_pos.x],
                                     &tiles[base_pos.y - 1][base_pos.x]};
      // I know I know, im checking can_move for every tile, too lazy to split
      // into 2 for loops
      if (!creature.can_move && tile.type == TileType::GREEN) {
        for (TileType *adj : adjacent_tiles) {
          if (*adj == TileType::NONE && rando() < GREEN_PRODUCTION_CHANCE) {
            *adj = TileType::FOOD;
          }
        }
      }
      if (tile.type == TileType::MOUTH) {
        for (TileType *adj : adjacent_tiles) {
          if (*adj == TileType::FOOD) {
            creature.food += 1;
            *adj = TileType::NONE;
          }
        }
      }
    }
    // handle movement
    if (creature.can_move) {
      bool blocked = false;
      const sf::Vector2i rel_vec = get_direction_vector(creature.movement);
      for (const Tile &tile : creature.tiles) {
        const sf::Vector2i abs_vec = rel_vec + creature.position + tile.rel_pos;
        if (this->tiles[abs_vec.y + abs_vec.y][abs_vec.x + abs_vec.x] !=
            TileType::NONE) {
          blocked = true;
          break;
        }
      }

      if (blocked || rando() < ROTATION_CHANCE) {
        const Direction new_dir = (Direction)(rando() * 4);
        const uint32_t diff =
            (uint32_t)new_dir - (uint32_t)creature.movement + 4 % 4;
        if (diff != 0) {
          bool rotation_blocked = false;
          Tile new_arr[MAX_TILES_PER_CREATURE];
          for (uint32_t i = 0; i < creature.num_tiles; i++) {
            Tile &tile = creature.tiles[i];
            const sf::Vector2i new_vec = rotate_vector(tile.rel_pos, diff);
            const sf::Vector2i abs_pos = creature.position + new_vec;
            if (this->tiles[abs_pos.y][abs_pos.x] != TileType::NONE) {
              rotation_blocked = true;
              break;
            } else
              new_arr[i] = {tile.type, new_vec};
          }

          if (!rotation_blocked)
            std::memcpy(creature.tiles, new_arr,
                        creature.num_tiles * sizeof(Tile));
        }
      }

      else {
        creature.position += rel_vec;
      }
    }

    //handle reproduction
    if (creature.food > creature.reproduction_cost) {
      Creature new_creature = creature.reproduce();

      bool spawn_blocked = false;
      uint32_t spawn_attempts = 0;
      do {
        sf::Vector2i offset = sf::Vector2i((int32_t)(rando() * (MAX_REPRODUCTION_DISTANCE * 2 + 1) - MAX_REPRODUCTION_DISTANCE), (int32_t)(rando() * (MAX_REPRODUCTION_DISTANCE * 2 + 1) - MAX_REPRODUCTION_DISTANCE));
        new_creature.position = creature.position + offset;

        for (Tile tile : new_creature.tiles) {
          sf::Vector2i tile_pos = new_creature.position + tile.rel_pos;
          if (this->tiles[tile_pos.y][tile_pos.x] != TileType::NONE) {
            spawn_blocked = true;
            break;
          }
        }
        spawn_attempts++;
      } while (spawn_blocked && spawn_attempts < MAX_SPAWN_ATTEMPTS);

      if (spawn_blocked) continue;

      new_creatures.push_back(new_creature);
      for (Tile tile : new_creature.tiles) {
        sf::Vector2i pos = new_creature.position + tile.rel_pos;

        this->tiles[pos.y][pos.x] = tile.type;
      }
      std::cout << "cloned creature (" << creature.position.x << ", " << creature.position.y << ")" << std::endl;
      std::cout << "new creature pos: (" << new_creature.position.x << ", " << new_creature.position.y << ")" << std::endl;
    }
  }

  this->creatures.reserve(this->creatures.size() + new_creatures.size());
  this->creatures.insert(this->creatures.end(), std::make_move_iterator(new_creatures.begin()), std::make_move_iterator(new_creatures.end()));
  std::this_thread::sleep_for(std::chrono::seconds(1));
}
