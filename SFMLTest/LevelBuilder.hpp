#pragma once

#include "Level.hpp"
#include "Enemy.hpp"
#include "Solver.hpp"

class LevelBuilder
{
public:
    static void build(Level& level, Enemy& enemy, Solver& particleSolver);

private:
    static void clearWorldState(Solver& particleSolver);
    static void setupBounds(Level& level);

    static void parseMap(Level& level, Enemy& enemy, Solver& particleSolver);
    static sf::Vector2f cellToWorld(
        const Level& level,
        int row,
        int col,
        float cellSize
    );

    static void spawnWall(const sf::Vector2f& pos, float size);
    static void spawnEnemy(Level& level, const sf::Vector2f& pos, Enemy::Type type);
    static void spawnParticles(Solver& particleSolver, const sf::Vector2f& pos, int count);
};
