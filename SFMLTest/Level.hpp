#pragma once

#include <vector>
#include <string>
#include <memory>
#include <SFML/Graphics.hpp>

#include "Item.hpp"
#include "Enemy.hpp"
#include "Player.hpp"
#include "Solver.hpp"


class Level {
public:
    bool customMapFile = false;
    std::string customMapFileName;

    sf::RectangleShape bounds;
    std::vector<Enemy> enemies;
    std::vector<std::unique_ptr<Item>> items;

    std::vector<std::string> mapFiles = { "lvl1.txt", "lvl2.txt", "lvl3.txt" };
    int currentMapIndex = 0;
    bool requestCloseRender = false;

    char** map = nullptr;
    int rows = 0;
    int cols = 0;

    void load(Player& playa, Enemy& enemy, Solver& particleSolver);
    void reset(Player& playa, Enemy& enemy, Solver& particleSolver);
    void freeMap();

    int getTotalTreasures() const;
    int getCollectedTreasures() const;
    void resetCollectedTreasures();

    int& addCollectedToTotal(Player& playa);
};
