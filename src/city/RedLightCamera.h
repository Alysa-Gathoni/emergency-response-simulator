#pragma once
#include <SFML/Graphics.hpp>
#include "RoadNetwork.h"
#include "TrafficLight.h"
#include "../lines/LineRenderer.h"

// Detects the ambulance crossing a stop line while that approach is red.
// The emergency vehicle is never forced to stop, but every red light it
// runs is counted so the assessment can weigh speed against safety.
class RedLightCamera {
public:
    RedLightCamera(const RoadNetwork& network, const TrafficLights& lights);

    // Call once per frame with the ambulance centre before and after moving.
    // Returns true when a red light was run during this step.
    bool update(Point previous, Point current, float deltaTime);

    int violations() const;
    void reset();

    void draw(sf::RenderTarget& target, const LineRenderer& lines) const;

private:
    const RoadNetwork& network_;
    const TrafficLights& lights_;
    int violations_{0};
    float flash_{0.0f};       // seconds of flash left
    Point flashAt_{};
};
