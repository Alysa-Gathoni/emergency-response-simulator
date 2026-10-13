#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "TrafficVehicle.h"

class TrafficSystem {
public:
    TrafficSystem();

    void update(float deltaTime);
    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool curveDebug) const;

private:
    std::vector<TrafficVehicle> vehicles_;
};
