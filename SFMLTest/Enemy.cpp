#include "Enemy.hpp"
#include "Player.hpp"
#include "Wall.hpp"
#include "MathUtils.hpp"

#include <cstdlib>
#include <cmath>
//Constructor
Enemy::Enemy(sf::Vector2f startPos_, int id_, Type t)
    : type(t), id(id_), pos(startPos_), startPos(startPos_)
{
    shape.setSize({ 20.f, 20.f });
    shape.setFillColor(
        type == Type::Moving ? sf::Color::Red : sf::Color::Blue
    );
    shape.setPosition(pos);

    // Random offset inside map for following enemies
    if (type == Type::Moving) {
        float maxOffset = 10.f;
        randomOffset = {
            static_cast<float>((rand() % static_cast<int>(maxOffset)) - maxOffset / 2.f),
            static_cast<float>((rand() % static_cast<int>(maxOffset)) - maxOffset / 2.f)
        };
    }
}

void Enemy::update(const std::vector<Wall>&,
    const sf::RectangleShape& worldBounds)
{
    // Clamp enemy inside world bounds
    pos = clampInsideRect(pos, shape.getSize(), worldBounds);
    shape.setPosition(pos);
}

void Enemy::updateAI(float dt, Player& player)
{
    static sf::Clock clock;
    float time = clock.getElapsedTime().asSeconds();

    sf::Vector2f playerPos = player.getPosition();

    // too types of enemies osciallting and following
    if (type == Type::Oscillating) {
        pos.y = startPos.y +
            std::sin(time * oscillationSpeed) * oscillationAmplitude;
    }
    else {
        float wiggleStrength = 80.f;

        sf::Vector2f noiseOffset(
            std::sin(time + id) * wiggleStrength,
            std::cos(time + id) * wiggleStrength
        );

        sf::Vector2f target = playerPos + noiseOffset;
        sf::Vector2f dir = target - pos;

        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.f)
            dir /= len;

        pos += dir * speed * dt;
    }

    shape.setPosition(pos);

    // Damage player if close
    float distance = std::sqrt(
        std::pow(playerPos.x - pos.x, 2.f) +
        std::pow(playerPos.y - pos.y, 2.f)
    );

    if (distance <= 30.f &&
        damageClock.getElapsedTime().asSeconds() >= damageCooldown)
    {
        player.takeDamage(5);
        damageClock.restart();
    }
}

void Enemy::draw(sf::RenderWindow& window) const
{
    window.draw(shape);
}
