#include "TrafficSystem.h"

TrafficSystem::TrafficSystem() {
    // Closed loop through the central road network, authored directly
    // as cubic Bezier segments.
    SplinePath loop;

    loop.addSegment({
        {80,285}, {190,285}, {275,285}, {390,285}
    });
    loop.addSegment({
        {390,285}, {455,285}, {445,345}, {510,345}
    });
    loop.addSegment({
        {510,345}, {620,345}, {730,345}, {820,345}
    });
    loop.addSegment({
        {820,345}, {850,345}, {850,325}, {820,285}
    });
    loop.addSegment({
        {820,285}, {620,285}, {300,285}, {80,285}
    });

    // Speeds are in pixels per second.
    vehicles_.emplace_back(loop, 85.0f, 0.00f);
    vehicles_.emplace_back(loop, 70.0f, 0.46f);

    // Vertical route: a closed Catmull-Rom spline through knots placed in
    // the two lanes, turning around at the top and bottom of the map.
    const SplinePath vertical = SplinePath::fromCatmullRom({
        {430,40}, {430,350}, {430,630}, {410,672},
        {390,630}, {390,350}, {390,70}, {410,22}
    }, true);

    vehicles_.emplace_back(vertical, 55.0f, 0.20f);
}

void TrafficSystem::update(float deltaTime) {
    for (auto& vehicle : vehicles_)
        vehicle.update(deltaTime);
}

void TrafficSystem::draw(sf::RenderTarget& target,
                         const LineRenderer& lines,
                         bool curveDebug) const {
    for (const auto& vehicle : vehicles_)
        vehicle.draw(target, lines, curveDebug);
}
