#pragma once
#include <SFML/Graphics.hpp>
#include "../core/Point.h"

class EmergencyMarker {
public:
    explicit EmergencyMarker(Point position);

    void update(float deltaTime);
    void draw(sf::RenderTarget& target, bool debug) const;

    Point position() const;
    void setPosition(Point position);
    void setColour(sf::Color colour);

private:
    Point position_;
    float phase_{0.0f};
    sf::Color colour_{235, 70, 70};
};
