#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

#include "Entity.hpp"

// Forward declarations
class Player;
class Wall;

class Enemy : public Entity {
public:
    enum class Type { Moving, Oscillating };

    // Constructor
    Enemy(sf::Vector2f startPos,
        int id,
        Type type = Type::Moving);

    // Required by Entity (movement + collisions)
    void update(const std::vector<Wall>& walls,
        const sf::RectangleShape& worldBounds) override;

    // Enemy-specific logic (chasing + damage)
    void updateAI(float dt, Player& player);

    // Rendering
    void draw(sf::RenderWindow& window) const;

    // Enemy.hpp
    void setColor(const sf::Color& color) { shape.setFillColor(color); }


private:
    Type type;
    int id;

    sf::Vector2f pos;
    sf::Vector2f startPos;

    sf::RectangleShape shape;
    float speed = 70.f;

    // Damage handling
    float damageCooldown = 1.f;
    sf::Clock damageClock;

    // Random movement offset
    sf::Vector2f randomOffset;

    // Oscillation
    float oscillationAmplitude = 50.f;
    float oscillationSpeed = 2.f;
};
