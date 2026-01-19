#pragma once
#include <SFML/Graphics.hpp>

// Utility function to keep a rectangle inside bounds
sf::Vector2f clampInsideRect(
    const sf::Vector2f& position,
    const sf::Vector2f& size,
    const sf::RectangleShape& bounds
);
