#include "HallOfFame.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

HallOfFame::HallOfFame(const std::string& file)
    : filename(file) {
}

void HallOfFame::load() {
    entries.clear();

    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);

        std::string name;
        std::string levelStr, treasuresStr, deathsStr;

        if (std::getline(iss, name, ',') &&
            std::getline(iss, levelStr, ',') &&
            std::getline(iss, treasuresStr, ',') &&
            std::getline(iss, deathsStr, ',')) {

            entries.emplace_back(
                name,
                std::stoi(levelStr),
                std::stoi(treasuresStr),
                std::stoi(deathsStr)
            );
        }
    }
}

void HallOfFame::save() const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    for (const auto& entry : entries) {
        file << entry.name << ","
            << entry.levelIndex << ","
            << entry.treasuresCollected << ","
            << entry.deaths << "\n";
    }
}

void HallOfFame::addEntry(const HallOfFameEntry& entry) {
    entries.push_back(entry);

    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) return;

    file << entry.name << ","
        << entry.levelIndex << ","
        << entry.treasuresCollected << ","
        << entry.deaths << "\n";
}

void HallOfFame::display() const {
    std::cout << "\n=== Hall of Fame Workers of The Month ===\n";

    for (const auto& e : entries) {
        std::cout << e.name
            << " | Level " << e.levelIndex
            << " | Minerals: " << e.treasuresCollected
            << " | Deaths: " << e.deaths << "\n";
    }

    std::cout << "===========================\n";
}
