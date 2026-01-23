#include "Level.hpp"
#include <fstream>
#include <iostream>
#include "LevelBuilder.hpp"

// ------------------- Treasure helpers -------------------

int Level::getTotalTreasures() const {
    return static_cast<int>(items.size());
}

int Level::getCollectedTreasures() const {
    int count = 0;
    for (const auto& item : items) {
        if (item->collected)
            ++count;
    }
    return count;
}

void Level::resetCollectedTreasures() {
    for (auto& item : items) {
        item->collected = false;
    }
}

// ------------------- Player interaction -------------------

int& Level::addCollectedToTotal(Player& player) {
    return player.totalTreasuresCollected;
}

// ------------------- Level load delete and reset -------------------

void Level::freeMap()
{
    if (map != nullptr) {
        for (int i = 0; i < rows; ++i)
            delete[] map[i];

        delete[] map;
        map = nullptr;
    }

    rows = 0;
    cols = 0;

    enemies.clear();
    items.clear();
}

// ------------------- Updated functions -------------------

void Level::load(GameData& gameData, Enemy& enemy)
{
    // Reset player & treasures
    reset(gameData, enemy);

    // Determine filename
    std::string filename;
    if (customMapFile) {
        filename = customMapFileName;
    }
    else {
        if (currentMapIndex < 0 || currentMapIndex >= mapFiles.size()) {
            std::cerr << "Invalid map index\n";
            return;
        }
        filename = mapFiles[currentMapIndex];
    }

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open map file: " << filename << "\n";
        return;
    }

    std::vector<std::string> contents;
    std::string line;
    while (std::getline(file, line))
        contents.push_back(line);

    if (contents.empty()) {
        std::cerr << "Empty map file\n";
        return;
    }

    rows = static_cast<int>(contents.size());
    cols = static_cast<int>(contents[0].size());

    // Allocate map
    map = new char* [rows];
    for (int i = 0; i < rows; ++i) {
        map[i] = new char[cols];
        for (int j = 0; j < cols; ++j)
            map[i][j] = contents[i][j];
    }

    // Build level using GameData
    LevelBuilder::build(*this, enemy, gameData);
}

void Level::reset(GameData& gameData, Enemy& enemy)
{
    // Reset player & treasures
    gameData.player.reset();
    resetCollectedTreasures();

    // Free previous map
    freeMap();

    // Clear walls/items/enemies/particles
    gameData.walls.clear();
    enemies.clear();
    items.clear();
    gameData.particleSolver.getObjects().clear();
}
