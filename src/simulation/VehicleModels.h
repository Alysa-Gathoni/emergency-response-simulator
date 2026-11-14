#pragma once
#include <SFML/Graphics.hpp>
#include "VehicleType.h"
#include "../core/Mat3.h"
#include "../lines/LineRenderer.h"

// Everything needed to draw one civilian vehicle. Models are authored in
// local coordinates (front of the vehicle along +x, centre at the origin)
// and placed with a single matrix: world = T(position) * R(heading).
struct VehiclePose {
    Mat3 world;
    VehicleType type;
    int palette;
    bool braking;
    int indicator;     // -1 left, 0 none, +1 right
    bool blinkOn;      // indicator lamp phase
};

namespace VehicleModels {
    void draw(sf::RenderTarget& target, const LineRenderer& lines,
              const VehiclePose& pose);
}
