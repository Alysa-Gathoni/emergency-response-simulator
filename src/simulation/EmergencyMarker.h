#pragma once
#include <SFML/Graphics.hpp>
#include "../core/Point.h"

class EmergencyMarker {
public:
    explicit EmergencyMarker(Point position);

    void update(float deltaTime);
    void draw(sf::RenderTarget& target, bool debug) const;

    Point position() const;

private:
    Point position_;
    float phase_{0.0f};
};
