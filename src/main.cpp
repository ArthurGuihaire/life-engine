#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <constants.hpp>
#include <map.hpp>
#include <optional>
#include <renderer.hpp>
#include <thread>
#include <utils.hpp>
#include <iostream>

struct ProfilePoint {
  uint32_t num_creatures;
  double update_time;
};

struct Instance {
  bool limit_fps = false;
  uint32_t fps = 5;
  std::chrono::time_point<std::chrono::steady_clock> next_frame_time = std::chrono::steady_clock::now();
  bool running = true;
  std::mutex instance_mutex;
  Map map;
  Renderer renderer;
};

void updateThread(Instance& instance) {
  while (instance.running) {
    instance.map.update();
    std::lock_guard<std::mutex> lock(instance.instance_mutex);
    if (instance.limit_fps) {
      auto now = std::chrono::steady_clock::now();
      if (now < instance.next_frame_time) {
        std::this_thread::sleep_until(instance.next_frame_time);
      }
      instance.next_frame_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000 / instance.fps);
    }
  }
}

void renderThread(Instance& instance) {
  while (instance.running) {
    while (const std::optional event = instance.renderer.window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        instance.renderer.window.close();
        std::lock_guard<std::mutex> lock(instance.instance_mutex);
        instance.running = false;
      }
      else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
      {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
          instance.renderer.window.close();
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num1) {
          std::lock_guard<std::mutex> lock(instance.instance_mutex);
          instance.limit_fps = true;
          instance.fps = 2;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num2) {
          std::lock_guard<std::mutex> lock(instance.instance_mutex);
          instance.limit_fps = true;
          instance.fps = 5;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num3) {
          std::lock_guard<std::mutex> lock(instance.instance_mutex);
          instance.limit_fps = true;
          instance.fps = 10;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num4) {
          std::lock_guard<std::mutex> lock(instance.instance_mutex);
          instance.limit_fps = true;
          instance.fps = 60;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num5) {
          std::lock_guard<std::mutex> lock(instance.instance_mutex);
          instance.limit_fps = false;
        }
      }
    }
    instance.renderer.render();
  }
}

int main() {
  random_generator::initialize();
  // Tile starting_tiles[3] = {{TileType::GREEN, {0, 0}},
  //                           {TileType::MOUTH, {-1, -1}},
  //                           {TileType::MOUTH, {1, 1}}};
  Tile starting_tiles[3] = {{TileType::GREEN, {0, 0}},
                            {TileType::MOUTH, {-1, -1}},
                            {TileType::MOUTH, {1, 1}}};
  Creature starting_creature(starting_tiles, 3,
                             {MAP_WIDTH / 2, MAP_HEIGHT / 2});

  sf::RenderWindow window(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}), "SFML Window");

  Instance instance {
    .map = Map(starting_creature),
    .renderer = Renderer(instance.map, window),
  };

  std::thread update_thread(updateThread, std::ref(instance));
  renderThread(instance);

  update_thread.join();

  /*
  while (renderer.window.isOpen()) {
    if (RUN_PROFILING) {
      auto start_time = std::chrono::steady_clock::now();
      map.update();
      auto end_time = std::chrono::steady_clock::now();
      profiling_data.push_back({map.numCreatures(), std::chrono::duration<double>(end_time - start_time).count()});
    }
    else {
      map.update();
    }
    renderer.render();
    while (const std::optional event = renderer.window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        renderer.window.close();
      }
      else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
      {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
          renderer.window.close();
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num1) {
          limit_fps = true;
          fps = 2;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num2) {
          limit_fps = true;
          fps = 5;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num3) {
          limit_fps = true;
          fps = 10;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num4) {
          limit_fps = true;
          fps = 60;
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Num5) {
          limit_fps = false;
        }
      }
    }
    std::chrono::time_point<std::chrono::steady_clock> current_frame_time = std::chrono::steady_clock::now();
    if (limit_fps && current_frame_time < next_frame_time) {
      std::this_thread::sleep_until(next_frame_time);
    }
    next_frame_time = std::chrono::time_point_cast<std::chrono::steady_clock::duration>(
        current_frame_time + std::chrono::duration<double>(1.0 / fps)
    );
  }

  if (RUN_PROFILING) {
    for (const auto& point : profiling_data) {
      std::cout << "(" << point.num_creatures << ", " << point.update_time << ")\n";
    }
  }
  */
}
