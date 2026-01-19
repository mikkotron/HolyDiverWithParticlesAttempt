#pragma once

#include <SFML/Graphics.hpp>
#include "Player.hpp" // include the Player class for applyEffect

/// <summary>
/// Base class for collectible items
/// </summary>
class Item {
public:
    sf::Vector2f position;
    sf::RectangleShape shape;
    bool collected = false;

    // Constructor
    Item(sf::Vector2f pos, sf::Color color, float size = 15.f);

    // Virtual destructor to allow proper polymorphic cleanup
    virtual ~Item() = default;

    // Effect applied when player collects the item
    virtual void applyEffect(Player& player) = 0;
};
