#pragma once

#include <SFML/Graphics.hpp>

class Wall {
public:
    sf::RectangleShape shape;

    // Constructor
    Wall(float x, float y, float width, float height);

    // Draw wall to render window
    void draw(sf::RenderWindow& window) const;
};
