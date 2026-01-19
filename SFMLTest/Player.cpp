#include "Player.hpp"
#include "Wall.hpp"
#include "MathUtils.hpp"  // for clampInsideRect
#include <iostream>
#include <cmath>

// Constructor
Player::Player(float sizeX, float sizeY) {
    rect.setSize({ sizeX, sizeY });
    rect.setOrigin(rect.getSize() / 2.f); // Optional: center origin
    up = down = left = right = false;
}

// Position / color
void Player::setPosition(const sf::Vector2f& pos) {
    rect.setPosition(pos);
}

void Player::setFillColor(const sf::Color& color) {
    rect.setFillColor(color);
}

// Input handling
void Player::handleInput(sf::Keyboard::Key key, bool pressed) {
    switch (key) {
    case sf::Keyboard::Key::W: up = pressed; break;
    case sf::Keyboard::Key::S: down = pressed; break;
    case sf::Keyboard::Key::A: left = pressed; break;
    case sf::Keyboard::Key::D: right = pressed; break;
    default: break;
    }
}

// Update
void Player::update(const std::vector<Wall>& walls,
    const sf::RectangleShape& worldBounds)
{
    velocity = { 0.f, 0.f };

    if (up)    velocity.y -= 2.f;
    if (down)  velocity.y += 2.f;
    if (left)  velocity.x -= 2.f;
    if (right) velocity.x += 2.f;

    rect.move(velocity);

    rect.setPosition(
        clampInsideRect(rect.getPosition(), rect.getSize(), worldBounds)
    );

    resolveCollisions(walls);
}

// Draw
void Player::draw(sf::RenderWindow& window) const {
    window.draw(rect);
}

// Collision
bool Player::checkCollision(const sf::RectangleShape& wallShape) {
    sf::Vector2f playerPos = rect.getPosition();
    sf::Vector2f playerHalf = rect.getSize() / 2.f;
    sf::Vector2f wallPos = wallShape.getPosition();
    sf::Vector2f wallHalf = wallShape.getSize() / 2.f;

    return (std::abs(playerPos.x - wallPos.x) < (playerHalf.x + wallHalf.x)) &&
        (std::abs(playerPos.y - wallPos.y) < (playerHalf.y + wallHalf.y));
}

void Player::resolveCollisions(const std::vector<Wall>& walls) {
    sf::Vector2f playerPos = rect.getPosition();
    sf::Vector2f playerHalf = rect.getSize() / 2.f;

    for (const auto& wall : walls) {
        sf::Vector2f wallPos = wall.shape.getPosition();
        sf::Vector2f wallHalf = wall.shape.getSize() / 2.f;

        if (checkCollision(wall.shape)) {
            sf::Vector2f delta = playerPos - wallPos;
            float overlapX = (playerHalf.x + wallHalf.x) - std::abs(delta.x);
            float overlapY = (playerHalf.y + wallHalf.y) - std::abs(delta.y);

            if (overlapX < overlapY)
                playerPos.x += delta.x > 0 ? overlapX : -overlapX;
            else
                playerPos.y += delta.y > 0 ? overlapY : -overlapY;

            rect.setPosition(playerPos);
        }
    }
}

// Position getter
sf::Vector2f Player::getPosition() const {
    return rect.getPosition();
}

// Health
void Player::takeDamage(int damage) {
    health -= damage;
    if (health < 0) health = 0;

    if (health == 0 && onDeath) {
        onDeath(); // triggers the callback exactly once
    }

}

bool Player::isDead() const {
    return health <= 0;
}

// Reset
void Player::reset() {
    health = 100;
    oxygenTime = 30.f;
    up = down = left = right = false;
}
