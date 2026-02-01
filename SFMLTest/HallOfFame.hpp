#pragma once

#include <vector>
#include <string>
#include "HallOfFameEntry.hpp"
//Loads saves and displays the players name and score and deaths 
class HallOfFame {
public:
    explicit HallOfFame(const std::string& file = "hall_of_fame.txt");

    void load();
    void save() const;
    void addEntry(const HallOfFameEntry& entry);
    void display() const;

private:
    std::vector<HallOfFameEntry> entries;
    std::string filename;
};

