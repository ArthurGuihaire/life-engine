#pragma once
#include <creature.hpp>
#include <SFML/System/Vector2.hpp>
#include <random>

namespace random_generator {
  extern std::mt19937 gen;
  extern std::uniform_real_distribution<double> random_dist;

  void initialize();
}
// random and rand are taken, what can I say
double rando();

sf::Vector2i rotate_vector(const sf::Vector2i input,
                           const uint32_t num_rotations);


sf::Vector2i get_direction_vector(const Direction direction);
