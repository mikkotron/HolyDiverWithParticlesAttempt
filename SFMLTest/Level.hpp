#pragma once

#include <vector>
#include <string>
#include <memory>

#include <SFML/Graphics.hpp>

#include "Item.hpp"
#include "Enemy.hpp"
#include "Player.hpp"

// ------------------- Level -------------------
// Handles level data, maps, and collectibles
class Level {
public:
    bool customMapFile = false;
    std::string customMapFileName;

    sf::RectangleShape bounds; // world boundary rectangle
    std::vector<Enemy> enemies;
    std::vector<std::unique_ptr<Item>> items;

    std::vector<std::string> mapFiles = { "lvl1.txt", "lvl2.txt", "lvl3.txt" };
    int currentMapIndex = 0;
    bool requestCloseRender = false;

    // Treasure helpers
    int getTotalTreasures() const;
    int getCollectedTreasures() const;
    void resetCollectedTreasures();

    // Access player's total treasure count
    int& addCollectedToTotal(Player& playa);
};
