// Solver.cpp
#include "Solver.hpp"
#include <cmath>

//SpatialHashGrid dividing the particles in to smaller areas to check collisions
Solver::SpatialHashGrid::SpatialHashGrid(float cellSize_) : cellSize(cellSize_) {}

long long Solver::SpatialHashGrid::hash(int x, int y) const {
    return ((long long)x << 32) ^ (long long)y;
}

sf::Vector2i Solver::SpatialHashGrid::worldToCell(const sf::Vector2f& pos, const sf::Vector2f& rectTopLeft) const {
    sf::Vector2f relativePos = pos - rectTopLeft;
    return sf::Vector2i((int)std::floor(relativePos.x / cellSize), (int)std::floor(relativePos.y / cellSize));
}

void Solver::SpatialHashGrid::insert(const sf::Vector2f& pos, int index, const sf::Vector2f& rectTopLeft) {
    sf::Vector2i cell = worldToCell(pos, rectTopLeft);
    long long key = hash(cell.x, cell.y);
    grid[key].push_back(index);
}

void Solver::SpatialHashGrid::clear() {
    grid.clear();
}

// Solver
Solver::Solver() = default;

Particle& Solver::addObject(sf::Vector2f position, float radius) {
    Particle newParticle(position, radius);
    return objects.emplace_back(newParticle);
}

void Solver::update(const sf::RectangleShape& rect) {
    applyGravity();

    sf::Vector2f rectTopLeft = rect.getPosition() - rect.getOrigin();
    hashGrid.clear();

    for (int i = 0; i < objects.size(); ++i)
        hashGrid.insert(objects[i].position, i, rectTopLeft);

    for (int i = 0; i < 3; ++i) {
        for (auto& obj : objects) {
            obj.solveBoundaries(rect);
            obj.update(step_dt);
        }
        checkCollisionsSpatial(rect);
    }
}
// visualising grid
void Solver::drawGrid(const sf::RectangleShape& rect, sf::RenderWindow& window) {
    float cellSize = hashGrid.cellSize;
    sf::RectangleShape cellOutline;
    cellOutline.setSize(sf::Vector2f(cellSize, cellSize));
    cellOutline.setFillColor(sf::Color::Transparent);
    cellOutline.setOutlineColor(sf::Color(60, 60, 60, 120));
    cellOutline.setOutlineThickness(1.f);

    sf::Vector2f rectTopLeft = rect.getPosition() - rect.getOrigin();
    int startCol = 0;
    int endCol = static_cast<int>(std::ceil(rect.getSize().x / cellSize));
    int startRow = 0;
    int endRow = static_cast<int>(std::ceil(rect.getSize().y / cellSize));

    for (int x = startCol; x < endCol; x++) {
        for (int y = startRow; y < endRow; y++) {
            cellOutline.setPosition(rectTopLeft + sf::Vector2f(x * cellSize, y * cellSize));
            window.draw(cellOutline);
        }
    }
}

std::vector<Particle>& Solver::getObjects() { return objects; }

void Solver::applyGravity() {
    for (auto& obj : objects)
        obj.applyAcceleration(gravity);
}

// checks grid cells inside the map area detecting distances between particles and if close enough psuhes them away
void Solver::checkCollisionsSpatial(const sf::RectangleShape& rect) {
    sf::Vector2f rectTopLeft = rect.getPosition() - rect.getOrigin();
    sf::Vector2f rectBottomRight = rectTopLeft + rect.getSize();

    for (auto& [key, cellParticles] : hashGrid.grid) {
        int cx = static_cast<int>(key >> 32);
        int cy = static_cast<int>(key & 0xFFFFFFFF);

        float cellLeft = rectTopLeft.x + cx * hashGrid.cellSize;
        float cellRight = cellLeft + hashGrid.cellSize;
        float cellTop = rectTopLeft.y + cy * hashGrid.cellSize;
        float cellBottom = cellTop + hashGrid.cellSize;

        if (cellRight < rectTopLeft.x || cellLeft > rectBottomRight.x ||
            cellBottom < rectTopLeft.y || cellTop > rectBottomRight.y) continue;

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                long long neighborKey = hashGrid.hash(cx + dx, cy + dy);
                auto it = hashGrid.grid.find(neighborKey);
                if (it == hashGrid.grid.end()) continue;

                const auto& neighborParticles = it->second;
                for (int i : cellParticles) {
                    Particle& obj1 = objects[i];
                    for (int j : neighborParticles) {
                        if (j <= i) continue;
                        Particle& obj2 = objects[j];

                        sf::Vector2f v = obj1.position - obj2.position;
                        float dist = std::sqrt(v.x * v.x + v.y * v.y);
                        float min_dist = obj1.radius + obj2.radius;
                        if (dist < min_dist) {
                            sf::Vector2f n = dist > 0.0001f ? v / dist : sf::Vector2f(1.f, 0.f);
                            float delta = 0.8f * (min_dist - dist);
                            obj1.position += n * 0.5f * delta;
                            obj2.position -= n * 0.5f * delta;
                            float damping = 0.99f;
                            obj1.position_last = obj1.position - (obj1.position - obj1.position_last) * damping;
                            obj2.position_last = obj2.position - (obj2.position - obj2.position_last) * damping;
                        }
                    }
                }
            }
        }
    }
}
