#pragma once
#include <SFML/Graphics.hpp>
#include "../curves/Spline.h"
#include "../lines/LineRenderer.h"

class TrafficVehicle {
public:
    // speed is in pixels per second along the path.
    TrafficVehicle(SplinePath path, float speed, float initialProgress = 0.0f);

    void update(float deltaTime);

    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool curveDebug) const;

private:
    SplinePath path_;
    float speed_;
    float progress_;
};
