#include <rando.hpp>
#include <random>

namespace random_generator {
  std::mt19937 gen;
  std::uniform_real_distribution<double> random_dist;

  void initialize() {
    // why is randomness in C++ so goddamn annoying
    std::random_device device;
    gen = std::mt19937(device());
  }
}

double rando() {
  return random_generator::random_dist(random_generator::gen);
}
