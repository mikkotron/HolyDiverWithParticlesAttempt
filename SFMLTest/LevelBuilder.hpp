#pragma once

#include "Level.hpp"
#include "Enemy.hpp"
#include "GameData.hpp" // <-- include GameData for player, walls, solver

class LevelBuilder
{
public:
    // Build the level using GameData instead of globals
    static void build(Level& level, Enemy& enemy, GameData& gameData);

private:
    // Internal helpers
    static void clearWorldState(GameData& gameData);
    static void setupBounds(Level& level);

    static void parseMap(Level& level, Enemy& enemy, GameData& gameData);
    static sf::Vector2f cellToWorld(
        const Level& level,
        int row,
        int col,
        float cellSize
    );

    static void spawnWall(GameData& gameData, const sf::Vector2f& pos, float size);
    static void spawnEnemy(Level& level, const sf::Vector2f& pos, Enemy::Type type);
    static void spawnParticles(GameData& gameData, const sf::Vector2f& pos, int count);
};
