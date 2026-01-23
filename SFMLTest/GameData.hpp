#pragma once
#include <vector>
#include "Player.hpp"
#include "Wall.hpp"
#include "Solver.hpp"

// Holds all shared game state previously in globals
struct GameData {
    // Player
    Player player{ 20.f, 20.f };

    // Particle physics solver
    Solver particleSolver;

    // Collection of walls
    std::vector<Wall> walls;

    // Flags
    bool isRendering = false;  // replaces openParticleSim
};

