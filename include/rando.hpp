#include <random>

namespace random_generator {
  extern std::mt19937 gen;
  extern std::uniform_real_distribution<double> random_dist;

  void initialize();
}
// random and rand are taken, what can I say
double rando();
