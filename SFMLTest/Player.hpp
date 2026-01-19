#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Entity.hpp"
#include <functional>


// Forward declaration
class Wall;

class Player : public Entity {
public:
    float oxygenTime = 30.f;
    int deaths = 0;
    int totalTreasuresCollected = 0;

    // Constructor
    Player(float sizeX = 20.f, float sizeY = 20.f);

    // Position and color
    void setPosition(const sf::Vector2f& pos);
    void setFillColor(const sf::Color& color);

    // Input handling
    void handleInput(sf::Keyboard::Key key, bool pressed);

    // Update and drawing
    void update(const std::vector<Wall>& walls, const sf::RectangleShape& worldBounds) override;
    void draw(sf::RenderWindow& window) const;

    // Collision and state
    sf::Vector2f getPosition() const;
    std::function<void()> onDeath;  // callback when player dies
    void takeDamage(int damage);
    bool isDead() const;
    void reset();

private:
    void resolveCollisions(const std::vector<Wall>& walls);
    bool checkCollision(const sf::RectangleShape& wallShape);

    sf::RectangleShape rect;
    sf::Vector2f velocity;
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};
