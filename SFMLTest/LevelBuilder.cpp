#include "LevelBuilder.hpp"
#include "Items.hpp"
#include "Solver.hpp"
#include "Wall.hpp"

#include <fstream>
#include <cstdlib> // rand()
#include <cstdlib> // rand()

// External systems (as in your project)
extern Solver particleSolver;
extern std::vector<Wall> walls;
extern Player playa;

// ----------------------------------------------------

void LevelBuilder::build(Level& level, Enemy& enemy, Solver& particleSolver)
{
    clearWorldState(particleSolver);
    setupBounds(level);
    parseMap(level, enemy, particleSolver);
}

// ----------------------------------------------------

void LevelBuilder::clearWorldState(Solver& particleSolver)
{
    particleSolver.getObjects().clear();
    walls.clear();
}

// ----------------------------------------------------

void LevelBuilder::setupBounds(Level& level)
{
    const float cellSize = 20.f;
    const float width = 840.f;
    const float height = 840.f;

    level.bounds.setSize({
        level.cols * cellSize,
        level.rows * cellSize
        });

    level.bounds.setOrigin(level.bounds.getSize() / 2.f);
    level.bounds.setPosition(sf::Vector2f(width / 2.f, height / 2.f));


    level.bounds.setFillColor(sf::Color::Black);
    level.bounds.setOutlineThickness(5.f);
    level.bounds.setOutlineColor(sf::Color::Blue);
}

// ----------------------------------------------------

void LevelBuilder::parseMap(Level& level, Enemy& enemy, Solver& particleSolver)
{
    const float cellSize = 20.f;
    const int particlesPerCell = 3;

    for (int row = 0; row < level.rows; ++row)
    {
        for (int col = 0; col < level.cols; ++col)
        {
            char cell = level.map[row][col];
            sf::Vector2f pos = cellToWorld(level, row, col, cellSize);

            switch (cell)
            {
            case 'x': spawnWall(pos, cellSize); break;
            case 'B': level.items.push_back(std::make_unique<HydraMineral>(pos)); break;
            case 'O': level.items.push_back(std::make_unique<Oxygen>(pos)); break;
            case 'o': spawnParticles(particleSolver, pos, particlesPerCell); break;
            case 'P': playa.setPosition(pos); break;
            case 'E': spawnEnemy(level, pos, Enemy::Type::Moving); break;
            case 'S': spawnEnemy(level, pos, Enemy::Type::Oscillating); break;
            default: break;
            }
        }
    }
}

// ----------------------------------------------------

sf::Vector2f LevelBuilder::cellToWorld(
    const Level& level,
    int row,
    int col,
    float cellSize)
{
    sf::Vector2f topLeft =
        level.bounds.getPosition() - level.bounds.getSize() / 2.f;

    return {
        topLeft.x + col * cellSize + cellSize / 2.f,
        topLeft.y + row * cellSize + cellSize / 2.f
    };
}

// ----------------------------------------------------

void LevelBuilder::spawnWall(const sf::Vector2f& pos, float size)
{
    walls.emplace_back(pos.x, pos.y, size, size);
}

// ----------------------------------------------------

void LevelBuilder::spawnEnemy(Level& level, const sf::Vector2f& pos, Enemy::Type type)
{
    int id = static_cast<int>(level.enemies.size());
    level.enemies.emplace_back(pos, id, type);

    if (type == Enemy::Type::Oscillating)
        level.enemies.back().setColor(sf::Color(255, 105, 180));
}

// ----------------------------------------------------

void LevelBuilder::spawnParticles(Solver& particleSolver, const sf::Vector2f& pos, int count)
{
    for (int i = 0; i < count; ++i)
    {
        float offsetX = static_cast<float>(rand() % 16 - 8);
        float offsetY = static_cast<float>(rand() % 16 - 8);

        particleSolver.addObject(
            { pos.x + offsetX, pos.y + offsetY },
            7.f
        );
    }
}
