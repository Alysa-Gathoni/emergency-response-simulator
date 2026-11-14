#include "EmergencyMarker.h"
#include "../shapes/CircleEllipse.h"
#include <cmath>

EmergencyMarker::EmergencyMarker(Point position)
    : position_(position) {}

void EmergencyMarker::update(float deltaTime) {
    phase_ += deltaTime * 3.0f;
}

void EmergencyMarker::draw(IRenderer& target, bool debug) const {
    const float pulse = (std::sin(phase_) + 1.0f) * 0.5f;
    const int radius = 18 + static_cast<int>(pulse * 14.0f);

    ShapeAlgorithms::drawPoints(
        target,
        ShapeAlgorithms::midpointCircle(position_, radius),
        colour_,
        3.0f
    );

    ShapeAlgorithms::drawPoints(
        target,
        ShapeAlgorithms::filledCircle(position_, 7),
        Rgba(255,210,80),
        1.0f
    );

    if (debug) {
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::midpointEllipse(position_, 42, 22),
            Rgba(80,220,170),
            1.0f
        );
    }
}

Point EmergencyMarker::position() const {
    return position_;
}

void EmergencyMarker::setPosition(Point position) {
    position_ = position;
    phase_ = 0.0f;
}

void EmergencyMarker::setColour(Rgba colour) {
    colour_ = colour;
}
