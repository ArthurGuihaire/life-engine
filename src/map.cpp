#include <constants.hpp>
#include <utils.hpp>
#include <map.hpp>
#include <tile.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <sys/types.h>
#include <chrono>
#include <boost/container/small_vector.hpp>

bool Map::collision(const sf::Vector2i& pos) {
  return pos.x < 0 || pos.x >= MAP_WIDTH || pos.y < 0 || pos.y >= MAP_HEIGHT || this->tiles[pos.y][pos.x] != TileType::NONE;
}

Map::Map(const Creature &starting_creature) : tiles{TileType::NONE} {
  creatures.push_back(starting_creature);
  writeToMap(starting_creature);
}

// uint8_t Map::getAdjacentTiles(const sf::Vector2i& pos, TileType* adjacent_tiles[4]) {
//   uint8_t mask = 0xFF;

//   for (uint32_t i = 0; i < sizeof(adjacent_offsets) / sizeof(adjacent_offsets[0]); i++) {
//     const sf::Vector2i adj_pos = pos + adjacent_offsets[i];
//     if (adj_pos.x < 0 || adj_pos.x >= MAP_WIDTH || adj_pos.y < 0 || adj_pos.y >= MAP_HEIGHT) {
//       mask &= ~(1 << i);
//     } else {
//       adjacent_tiles[i] = &tiles[adj_pos.y][adj_pos.x];
//     }
//   }

//   return mask;
// }

void Map::erase(const Creature& creature, const TileType type) {
  for (uint32_t idx = 0; idx < creature.num_tiles; idx++) {
    const Tile& tile = creature.tiles[idx];
    const sf::Vector2i base_pos = creature.position + tile.rel_pos;
    tiles[base_pos.y][base_pos.x] = type;
  }
}

void Map::writeToMap(const Creature& creature) {
  for (uint32_t idx = 0; idx < creature.num_tiles; idx++) {
    const Tile& tile = creature.tiles[idx];
    const sf::Vector2i base_pos = creature.position + tile.rel_pos;
    tiles[base_pos.y][base_pos.x] = tile.type;
  }
}

void Map::kill(const uint32_t idx) {
  this->erase(this->creatures[idx], TileType::FOOD);
  this->creatures[idx] = this->creatures.back();
  this->creatures.pop_back();
}

static std::vector<Creature> new_creatures;

void Map::update() {
  new_creatures.clear();
  uint32_t creatures_alive = this->creatures.size();
  for (uint32_t idx = 0; idx < creatures_alive; ++idx) {
    this->creatures[idx].update(*this);

    if (this->creatures[idx].food >= this->creatures[idx].reproduction_cost) {
      Creature new_creature = this->creatures[idx].reproduce();

      bool spawn_blocked = false;
      uint32_t spawn_attempts = 0;
      do {
        sf::Vector2i offset = sf::Vector2i((int32_t)(rando() * (MAX_REPRODUCTION_DISTANCE * 2 + 1) - MAX_REPRODUCTION_DISTANCE), (int32_t)(rando() * (MAX_REPRODUCTION_DISTANCE * 2 + 1) - MAX_REPRODUCTION_DISTANCE));
        new_creature.position = this->creatures[idx].position + offset;

        spawn_blocked = false;
        for (uint32_t tile_idx = 0; tile_idx < new_creature.num_tiles; ++tile_idx) {
          const Tile &tile = new_creature.tiles[tile_idx];
          const sf::Vector2i tile_pos = new_creature.position + tile.rel_pos;
          if (tile_pos.x < 0 || tile_pos.x >= MAP_WIDTH || tile_pos.y < 0 || tile_pos.y >= MAP_HEIGHT || this->tiles[tile_pos.y][tile_pos.x] != TileType::NONE) {
            spawn_blocked = true;
            break;
          }
        }
        spawn_attempts++;
      } while (spawn_blocked && spawn_attempts < MAX_SPAWN_ATTEMPTS);

      if (spawn_blocked) continue;

      new_creatures.push_back(new_creature);
      writeToMap(new_creature);
    }

    if (this->creatures[idx].age > MAX_AGE_FACTOR * this->creatures[idx].num_tiles) {
      this->kill(idx);
      creatures_alive--;
      continue;
    }

    this->creatures[idx].age++;
    for (uint32_t i = 0; i < this->creatures[idx].num_adjacent_tiles; i++) {
      if (this->tiles[this->creatures[idx].adjacent_tiles[i].y + this->creatures[idx].position.y][this->creatures[idx].adjacent_tiles[i].x +  this->creatures[idx].position.x] == TileType::KILL) {
        this->creatures[idx].health -= 1;
        if (this->creatures[idx].health <= 0) {
          creatures_alive--;
          this->kill(idx);
          break;
        }
      }
    }
  }

  this->creatures.insert(this->creatures.end(), std::make_move_iterator(new_creatures.begin()), std::make_move_iterator(new_creatures.end()));
  // std::this_thread::sleep_for(std::chrono::seconds(1));

  // this->next_update_time += UPDATE_INTERVAL;
  // std::this_thread::sleep_until(this->next_update_time);
  //
}

// yet another BFS
uint32_t Map::cascadeEat(const sf::Vector2i& position) {
  uint32_t num_eaten = 0;

  boost::container::small_vector<sf::Vector2i, MAX_CASCADES> cascade_stack;

  cascade_stack.push_back(position);
  while (!cascade_stack.empty()) {
    sf::Vector2i pos = cascade_stack.back();
    cascade_stack.pop_back();

    this->tiles[pos.y][pos.x] = TileType::NONE;
    num_eaten++;

    for (const sf::Vector2i& offset : adjacent_offsets) {
      if (num_eaten + cascade_stack.size() >= MAX_CASCADES) {
        return num_eaten;
      }
      const sf::Vector2i adj_pos = pos + offset;
      if (adj_pos.x < 0 || adj_pos.x >= MAP_WIDTH || adj_pos.y < 0 || adj_pos.y >= MAP_HEIGHT) {
        continue;
      }
      if (this->tiles[adj_pos.y][adj_pos.x] == TileType::FOOD) {
        cascade_stack.push_back(adj_pos);
      }
    }
  }

  return num_eaten;
}
