#include "Item.hpp"

// Constructor implementation
Item::Item(sf::Vector2f pos, sf::Color color, float size)
    : position(pos)
{
    shape.setSize({ size, size });
    shape.setFillColor(color);
    shape.setPosition(position);
}
