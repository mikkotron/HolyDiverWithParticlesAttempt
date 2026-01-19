#pragma once
#include <SFML/Graphics.hpp>

// Base class for all game entities
class Entity {
public:
    virtual ~Entity() = default;

    int health = 100;
    sf::RectangleShape body;

    // Pure virtual update with walls & world bounds
    virtual void update(const std::vector<class Wall>& walls,
        const sf::RectangleShape& worldBounds) = 0;
};

