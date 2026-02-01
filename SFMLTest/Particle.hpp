
#pragma once

#include <SFML/Graphics.hpp>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f position_last;
    sf::Vector2f acceleration;
    float radius = 6.0f;
    sf::CircleShape shape;

    // Constructors
    Particle();
    Particle(sf::Vector2f position_, float radius_);

    // Main update loop
    void update(float dt);

    // Physics
    void applyAcceleration(sf::Vector2f a);
    void setVelocity(sf::Vector2f v, float dt);
    void addVelocity(sf::Vector2f v, float dt);
    sf::Vector2f getVelocity() const;

    // Collision / bounds
    void solveBoundaries(const sf::RectangleShape& rect);
};
