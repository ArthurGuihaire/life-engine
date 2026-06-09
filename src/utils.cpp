#include <SFML/System/Vector2.hpp>
#include <utils.hpp>
#include <random>
#include <tile.hpp>

namespace random_generator {
  std::mt19937 gen;
  std::uniform_real_distribution<double> random_dist;

  void initialize() {
    // why is randomness in C++ so goddamn annoying
    std::random_device device;
    gen = std::mt19937(device());
    random_dist = std::uniform_real_distribution<double>(0.0, 1.0);
  }
}

double rando() {
  return random_generator::random_dist(random_generator::gen);
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
