#include "TrafficVehicle.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include "../shapes/CircleEllipse.h"
#include "../core/Color.h"
#include <cmath>

namespace {
constexpr float PI = 3.14159265358979323846f;

Point transformLocal(Point centre, float angleRadians, float lx, float ly) {
    const float c = std::cos(angleRadians);
    const float s = std::sin(angleRadians);
    return {
        centre.x + lx * c - ly * s,
        centre.y + lx * s + ly * c
    };
}
}

TrafficVehicle::TrafficVehicle(
    SplinePath path, float speed, float initialProgress
) : path_(std::move(path)),
    speed_(speed),
    progress_(initialProgress) {}

void TrafficVehicle::update(float deltaTime) {
    // Arc-length parameterisation: convert distance into path progress so
    // the speed stays constant on short and long segments alike.
    const float length = path_.length();
    if (length <= 0.0f) return;

    progress_ += speed_ * deltaTime / length;
    while (progress_ >= 1.0f) progress_ -= 1.0f;
    while (progress_ < 0.0f) progress_ += 1.0f;
}

void TrafficVehicle::draw(sf::RenderTarget& target,
                          const LineRenderer& lines,
                          bool curveDebug) const {
    if (path_.empty()) return;

    const Point centre = path_.evaluate(progress_);
    const Point direction = path_.tangent(progress_);
    const float angle = std::atan2(direction.y, direction.x);

    std::vector<Point> vertices = {
        transformLocal(centre, angle, -16, -9),
        transformLocal(centre, angle,  16, -9),
        transformLocal(centre, angle,  16,  9),
        transformLocal(centre, angle, -16,  9)
    };

    Polygon car(vertices);
    PolygonFill::draw(target, car.vertices(), lines,
                      sf::Color(70,135,205), false);
    car.drawOutline(target, lines, sf::Color(200,220,240), 2.0f);

    const Point frontWheel = transformLocal(centre, angle, 9, 11);
    const Point rearWheel  = transformLocal(centre, angle, -9, 11);

    ShapeAlgorithms::drawPoints(
        target, ShapeAlgorithms::filledCircle(frontWheel, 3),
        sf::Color(25,25,25), 1.0f);
    ShapeAlgorithms::drawPoints(
        target, ShapeAlgorithms::filledCircle(rearWheel, 3),
        sf::Color(25,25,25), 1.0f);

    if (!curveDebug) return;

    const auto pathPoints = path_.sampledPoints(50);
    for (std::size_t i = 1; i < pathPoints.size(); ++i) {
        lines.draw(target, pathPoints[i-1], pathPoints[i],
                   Colors::Debug, 1.0f);
    }

    for (const auto& segment : path_.segments()) {
        // Control polygon
        lines.draw(target, segment.p0, segment.p1,
                   sf::Color(140,140,140), 1.0f);
        lines.draw(target, segment.p1, segment.p2,
                   sf::Color(140,140,140), 1.0f);
        lines.draw(target, segment.p2, segment.p3,
                   sf::Color(140,140,140), 1.0f);

        for (Point p : {segment.p0, segment.p1, segment.p2, segment.p3}) {
            ShapeAlgorithms::drawPoints(
                target,
                ShapeAlgorithms::filledCircle(p, 4),
                sf::Color(235,190,70),
                1.0f
            );
        }
    }

    // Catmull-Rom knots: the path passes through each of these.
    for (Point knot : path_.knots()) {
        ShapeAlgorithms::drawPoints(
            target, ShapeAlgorithms::filledCircle(knot, 6),
            sf::Color(220,90,200), 1.0f);
    }
}
