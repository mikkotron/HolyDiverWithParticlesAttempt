#include "HydraMineral.hpp"
#include <algorithm>

HydraMineral::HydraMineral(sf::Vector2f pos)
    : Item(pos, sf::Color::Yellow)
{
}

void HydraMineral::applyEffect(Player& player) {
    player.health = std::min(player.health + 5, 100);
    collected = true;
}
