#include "EmergencyMarker.h"
#include "../shapes/CircleEllipse.h"
#include <cmath>

EmergencyMarker::EmergencyMarker(Point position)
    : position_(position) {}

void EmergencyMarker::update(float deltaTime) {
    phase_ += deltaTime * 3.0f;
}

void EmergencyMarker::draw(sf::RenderTarget& target, bool debug) const {
    const float pulse = (std::sin(phase_) + 1.0f) * 0.5f;
    const int radius = 18 + static_cast<int>(pulse * 14.0f);

    ShapeAlgorithms::drawPoints(
        target,
        ShapeAlgorithms::midpointCircle(position_, radius),
        sf::Color(235,70,70),
        3.0f
    );

    ShapeAlgorithms::drawPoints(
        target,
        ShapeAlgorithms::midpointCircle(position_, 7),
        sf::Color(255,210,80),
        2.0f
    );

    if (debug) {
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::midpointEllipse(position_, 42, 22),
            sf::Color(80,220,170),
            1.0f
        );
    }
}

Point EmergencyMarker::position() const {
    return position_;
}
