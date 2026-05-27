#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <map.hpp>

constexpr inline sf::Color colorMap[] = {
    sf::Color::Black,
    sf::Color({128, 128, 128, 255}),
    sf::Color::Green,
    sf::Color::Yellow,
    sf::Color::Red,
    sf::Color::Blue,
    sf::Color({255, 0, 255, 255}),
};

class Renderer {

    public:
        Renderer(const Map& map);
        void render();

        sf::RenderWindow window;
    private:
        const Map& global_map;
        sf::Texture mapTexture;
        sf::Sprite mapSprite;
};
