#include "Wall.hpp"

// Constructor
Wall::Wall(float x, float y, float width, float height) {
    shape.setSize({ width, height });
    shape.setOrigin(shape.getSize() / 2.f); // center origin
    shape.setFillColor(sf::Color::Blue);
    shape.setPosition({ x, y });
}

// Draw wall
void Wall::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}
