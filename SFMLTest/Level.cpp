#include "Level.hpp"

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

int& Level::addCollectedToTotal(Player& playa) {
    return playa.totalTreasuresCollected;
}
