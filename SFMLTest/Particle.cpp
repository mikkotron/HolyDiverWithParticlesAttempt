// Particle.cpp
#include "Particle.hpp"
#include <cmath>

// Default constructor
Particle::Particle() = default;

// Constructor with position and radius
Particle::Particle(sf::Vector2f position_, float radius_)
    : position(position_), position_last(position_), acceleration(0.f, 0.f), radius(radius_)
{
    shape.setRadius(radius);
    shape.setFillColor(sf::Color(0, 150, 255, 255));
    shape.setPosition(position);
}

// Update function (verlet integration)
void Particle::update(float dt) {
    float damping = 0.99f;
    sf::Vector2f displacement = (position - position_last) * damping;
    position_last = position;
    position = position + displacement + acceleration * (dt * dt);
    acceleration = {};
    shape.setPosition(position);
}

// Physics
void Particle::applyAcceleration(sf::Vector2f a) { acceleration += a; }

void Particle::setVelocity(sf::Vector2f v, float dt) { position_last = position - (v * dt); }

void Particle::addVelocity(sf::Vector2f v, float dt) { position_last -= v * dt; }

sf::Vector2f Particle::getVelocity() const { return position - position_last; }

// Solve boundaries
void Particle::solveBoundaries(const sf::RectangleShape& rect) {
    float r = shape.getRadius();
    float outline = rect.getOutlineThickness();

    // Compute top-left and bottom-right based on origin and size
    sf::Vector2f rectTopLeft = rect.getPosition() - rect.getOrigin() - sf::Vector2f(outline, outline);
    sf::Vector2f rectBottomRight = rectTopLeft + rect.getSize() + sf::Vector2f(outline * 2.f, outline * 2.f);

    float b = 0.95f; // bounce factor

    if (position.x - r < rectTopLeft.x) {
        position.x = rectTopLeft.x + r;
        position_last.x = position.x + (position_last.x - position.x) * -b;
    }
    if (position.x + r > rectBottomRight.x) {
        position.x = rectBottomRight.x - r;
        position_last.x = position.x + (position_last.x - position.x) * -b;
    }
    if (position.y - r < rectTopLeft.y) {
        position.y = rectTopLeft.y + r;
        position_last.y = position.y + (position_last.y - position.y) * -b;
    }
    if (position.y + r > rectBottomRight.y) {
        position.y = rectBottomRight.y - r;
        position_last.y = position.y + (position_last.y - position.y) * -b;
    }
}
