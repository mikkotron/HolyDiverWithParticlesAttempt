#include "MathUtils.hpp"
// making sure player and enemy wont get out from the map
sf::Vector2f clampInsideRect(
    const sf::Vector2f& position,
    const sf::Vector2f& size,
    const sf::RectangleShape& bounds
) {
    sf::Vector2f halfSize = size * 0.5f;

    sf::Vector2f topLeft =
        bounds.getPosition() - bounds.getSize() * 0.5f;
    sf::Vector2f bottomRight =
        bounds.getPosition() + bounds.getSize() * 0.5f;

    sf::Vector2f clamped = position;

    if (clamped.x - halfSize.x < topLeft.x)
        clamped.x = topLeft.x + halfSize.x;
    if (clamped.x + halfSize.x > bottomRight.x)
        clamped.x = bottomRight.x - halfSize.x;

    if (clamped.y - halfSize.y < topLeft.y)
        clamped.y = topLeft.y + halfSize.y;
    if (clamped.y + halfSize.y > bottomRight.y)
        clamped.y = bottomRight.y - halfSize.y;

    return clamped;
}
