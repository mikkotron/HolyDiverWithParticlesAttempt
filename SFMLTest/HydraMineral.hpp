#pragma once

#include "Item.hpp"

class HydraMineral : public Item {
public:
    explicit HydraMineral(sf::Vector2f pos);
    void applyEffect(Player& player) override;
};
