#pragma once

#include <string>

struct HallOfFameEntry {
    std::string name;
    int levelIndex = 0;
    int treasuresCollected = 0;
    int deaths = 0;

    HallOfFameEntry() = default;

    HallOfFameEntry(
        const std::string& n,
        int lvl,
        int treas,
        int d
    )
        : name(n),
        levelIndex(lvl),
        treasuresCollected(treas),
        deaths(d) {
    }
};

