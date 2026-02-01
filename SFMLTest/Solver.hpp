
#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include "Particle.hpp"

/// <summary>
/// Solver using spatial hash grid to resolve particle collisions efficiently.
/// </summary>
class Solver {
public:
    // Spatial hash grid for quick collision lookup
    struct SpatialHashGrid {
        float cellSize;
        std::unordered_map<long long, std::vector<int>> grid;

        SpatialHashGrid(float cellSize_);

        long long hash(int x, int y) const;
        sf::Vector2i worldToCell(const sf::Vector2f& pos, const sf::Vector2f& rectTopLeft) const;
        void insert(const sf::Vector2f& pos, int index, const sf::Vector2f& rectTopLeft);
        void clear();
    };

    Solver();

    // Add new particle
    Particle& addObject(sf::Vector2f position, float radius);

    // Main update loop
    void update(const sf::RectangleShape& rect);

    // Draw spatial hash grid (for debug)
    void drawGrid(const sf::RectangleShape& rect, sf::RenderWindow& window);

    // Access particles
    std::vector<Particle>& getObjects();

private:
    std::vector<Particle> objects;
    SpatialHashGrid hashGrid{ 45.0f };
    sf::Vector2f gravity{ 0.f, 800.f };
    float step_dt{ 1.0f / 120.f };

    void applyGravity();
    void checkCollisionsSpatial(const sf::RectangleShape& rect);
};
