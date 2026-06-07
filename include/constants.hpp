#pragma once
#include <SFML/System/Vector2.hpp>
#include <chrono>
#include <cstdint>

constexpr inline uint32_t SCREEN_WIDTH = 1920;
constexpr inline uint32_t SCREEN_HEIGHT = 1080;

constexpr inline uint32_t TILE_SIZE = 4;

constexpr inline uint32_t MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
constexpr inline uint32_t MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;

constexpr inline uint32_t MAX_TILES_PER_CREATURE = 32;

constexpr inline uint32_t MAX_REPRODUCTION_DISTANCE = 5;
constexpr inline uint32_t REPRODUCTION_FOOD_FACTOR = 1;
constexpr inline uint32_t REPRODUCTION_FOOD_BASE = 3;
constexpr inline uint32_t MAX_AGE_FACTOR = 50;
constexpr inline float GREEN_PRODUCTION_CHANCE = 0.05; // 0.04
constexpr inline uint32_t KILL_FOOD_BONUS = 0;

constexpr inline float ADD_TILE_MUTATION_CHANCE = 0.1;
constexpr inline float REMOVE_TILE_MUTATION_CHANCE =
    0.05 +
    ADD_TILE_MUTATION_CHANCE; // addition here just makes probabilities easier
constexpr inline float CHANGE_TILE_MUTATION_CHANCE =
    0.1 + REMOVE_TILE_MUTATION_CHANCE; // actual probability is 0.1
constexpr inline float ROTATION_CHANCE = 0.25;
constexpr inline uint32_t MAX_SPAWN_ATTEMPTS = 0;

constexpr inline sf::Vector2i adjacent_offsets[4] = {
    {1, 0}, {0, 1}, {-1, 0}, {0, -1}};

constexpr inline uint32_t NUM_TILE_TYPES = 7; // useful for random generation
constexpr inline uint32_t NUM_NONLIVING_TYPES = 2;

constexpr inline uint32_t UPDATES_PER_SECOND = 60;
constexpr inline std::chrono::microseconds UPDATE_INTERVAL = std::chrono::microseconds(1000000 / UPDATES_PER_SECOND);
