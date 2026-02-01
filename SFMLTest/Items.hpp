#pragma once

#include "Item.hpp"
#include <algorithm>
// classes for each collectible item and the effect functions
class HydraMineral : public Item {
public:
    explicit HydraMineral(sf::Vector2f pos)
        : Item(pos, sf::Color::Yellow) {
    }

    void applyEffect(Player& player) override {
        player.health = std::min(player.health + 5, 100);
        collected = true;
    }
};

class Oxygen : public Item {
public:
    explicit Oxygen(sf::Vector2f pos)
        : Item(pos, sf::Color::White) {
    }

    void applyEffect(Player& player) override {
        player.oxygenTime = std::min(player.oxygenTime + 10.f, 30.f);
        collected = true;
    }
};
