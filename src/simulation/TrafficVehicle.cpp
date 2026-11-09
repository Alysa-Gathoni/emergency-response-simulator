#include "TrafficVehicle.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include "../shapes/CircleEllipse.h"
#include "../core/Color.h"
#include <algorithm>
#include <cmath>

namespace {

Point transformLocal(Point centre, Point dir, float lx, float ly) {
    // Local frame: x along the direction of travel, y to its right.
    return {
        centre.x + lx * dir.x - ly * dir.y,
        centre.y + lx * dir.y + ly * dir.x
    };
}

sf::Color limiterColor(SpeedLimiter reason) {
    switch (reason) {
        case SpeedLimiter::Free:   return sf::Color(80, 220, 170);
        case SpeedLimiter::Corner: return sf::Color(90, 160, 240);
        case SpeedLimiter::Signal: return sf::Color(235, 70, 70);
        case SpeedLimiter::Leader: return sf::Color(240, 170, 60);
        case SpeedLimiter::Stage:  return sf::Color(220, 90, 200);
    }
    return sf::Color::White;
}

}

namespace {

const sf::Color CarColors[] = {
    {70, 135, 205}, {196, 64, 58}, {190, 194, 200}, {64, 150, 112}, {230, 230, 226}
};
const sf::Color MatatuColors[] = {
    {242, 196, 48}, {120, 70, 170}, {40, 170, 120}
};
const sf::Color RiderColors[] = {
    {230, 120, 40}, {40, 120, 230}, {220, 220, 60}
};

}

TrafficVehicle::TrafficVehicle(std::shared_ptr<const TrafficRoute> route,
                               VehicleType type, float cruiseSpeed,
                               float startDistance, int paletteIndex)
    : route_(std::move(route)),
      spec_(&specFor(type)),
      cruiseSpeed_(cruiseSpeed),
      distance_(startDistance),
      paletteIndex_(paletteIndex),
      speed_(cruiseSpeed * 0.5f),
      targetSpeed_(cruiseSpeed) {}

void TrafficVehicle::setTarget(float targetSpeed, SpeedLimiter reason,
                               Point blockingPoint) {
    targetSpeed_ = targetSpeed;
    limiter_ = reason;
    blockingPoint_ = blockingPoint;
}

void TrafficVehicle::integrate(float deltaTime) {
    if (dwell_ > 0.0f) {
        // Passengers boarding at a matatu stage.
        dwell_ -= deltaTime;
        speed_ = 0.0f;
        return;
    }

    // Speed approaches the target: gentle acceleration, firmer braking
    // (up to twice the comfortable value when something appears close).
    if (speed_ < targetSpeed_)
        speed_ = std::min(targetSpeed_, speed_ + spec_->acceleration * deltaTime);
    else
        speed_ = std::max(targetSpeed_, speed_ - 2.0f * spec_->braking * deltaTime);
    speed_ = std::max(0.0f, speed_);

    // Arc-length parameterisation: distance travelled maps straight onto
    // the path, so speed stays constant on short and long segments alike.
    const float length = route_->path.length();
    if (length <= 0.0f) return;
    distance_ = std::fmod(distance_ + speed_ * deltaTime, length);

    updateStage();
}

void TrafficVehicle::updateStage() {
    if (spec_->stageDwell <= 0.0f) return;

    const float length = route_->path.length();
    const auto& stages = route_->stages;
    for (int k = 0; k < static_cast<int>(stages.size()); ++k) {
        // Signed distance to the stage, wrapped into (-L/2, L/2].
        float d = stages[k] - distance_;
        if (d > length * 0.5f) d -= length;
        if (d <= -length * 0.5f) d += length;

        if (k == servedStage_) {
            // Ready to serve this stage again on the next lap.
            if (std::abs(d) > 100.0f) servedStage_ = -1;
            continue;
        }
        if (std::abs(d) < 3.0f && speed_ < 3.0f) {
            dwell_ = spec_->stageDwell;
            servedStage_ = k;
            speed_ = 0.0f;
        }
    }
}

const TrafficRoute& TrafficVehicle::route() const { return *route_; }
VehicleType TrafficVehicle::type() const { return spec_->type; }
const VehicleSpec& TrafficVehicle::spec() const { return *spec_; }
int TrafficVehicle::servedStage() const { return servedStage_; }
float TrafficVehicle::distance() const { return distance_; }
float TrafficVehicle::speed() const { return speed_; }
float TrafficVehicle::cruiseSpeed() const { return cruiseSpeed_; }
float TrafficVehicle::comfortableBraking() const { return spec_->braking; }
float TrafficVehicle::halfLength() const { return spec_->halfLength; }
float TrafficVehicle::halfWidth() const { return spec_->halfWidth; }

Point TrafficVehicle::position() const {
    return route_->path.pointAtDistance(distance_);
}

Point TrafficVehicle::direction() const {
    return route_->path.directionAtDistance(distance_);
}

std::vector<Point> TrafficVehicle::footprint() const {
    const Point c = position();
    const Point d = direction();
    const float hl = spec_->halfLength;
    const float hw = spec_->halfWidth;
    return {
        transformLocal(c, d, -hl, -hw),
        transformLocal(c, d,  hl, -hw),
        transformLocal(c, d,  hl,  hw),
        transformLocal(c, d, -hl,  hw)
    };
}

void TrafficVehicle::drawBody(sf::RenderTarget& target,
                              const LineRenderer& lines) const {
    const Point c = position();
    const Point d = direction();
    const float hl = spec_->halfLength;
    const float hw = spec_->halfWidth;
    auto local = [&](float lx, float ly) { return transformLocal(c, d, lx, ly); };
    auto disc = [&](Point p, int r, sf::Color col) {
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(p, r), col, 1.0f);
    };
    const sf::Color tyre(25, 25, 25);
    const sf::Color glass(40, 50, 64);
    const auto body = footprint();

    switch (spec_->type) {
    case VehicleType::Car:
        PolygonFill::draw(target, body, lines, CarColors[paletteIndex_ % 5], false);
        Polygon(body).drawOutline(target, lines, sf::Color(20, 24, 30), 1.0f);
        // Windscreen and rear window.
        lines.draw(target, local(5, -hw + 2), local(5, hw - 2), glass, 2.0f);
        lines.draw(target, local(-10, -hw + 2), local(-10, hw - 2), glass, 2.0f);
        break;

    case VehicleType::Matatu: {
        PolygonFill::draw(target, body, lines, MatatuColors[paletteIndex_ % 3], false);
        Polygon(body).drawOutline(target, lines, sf::Color(20, 24, 30), 1.0f);
        // Livery stripe along both sides, roof sign and windscreen.
        for (float side : {-hw + 3.0f, hw - 3.0f})
            lines.draw(target, local(-hl + 4, side), local(hl - 6, side),
                       sf::Color(200, 40, 60), 2.0f);
        const std::vector<Point> sign = {local(-6, -5), local(6, -5), local(6, 5), local(-6, 5)};
        PolygonFill::draw(target, sign, lines, sf::Color(250, 250, 250), false);
        lines.draw(target, local(hl - 7, -hw + 2), local(hl - 7, hw - 2), glass, 2.0f);
        break;
    }

    case VehicleType::Motorcycle:
        disc(local(hl - 4, 0), 4, tyre);
        disc(local(-hl + 4, 0), 4, tyre);
        PolygonFill::draw(target, body, lines, sf::Color(60, 60, 66), false);
        disc(local(-1, 0), 4, RiderColors[paletteIndex_ % 3]);   // helmet
        lines.draw(target, local(hl - 6, -hw - 2), local(hl - 6, hw + 2),
                   sf::Color(150, 150, 150), 1.0f);              // handlebar
        return;
    }

    for (float side : {-hw - 1.0f, hw + 1.0f})
        for (float along : {-hl * 0.6f, hl * 0.6f})
            disc(local(along, side), 3, tyre);
}

void TrafficVehicle::draw(sf::RenderTarget& target,
                          const LineRenderer& lines,
                          bool curveDebug,
                          bool trafficDebug) const {
    const SplinePath& path = route_->path;
    if (path.empty()) return;

    const Point centre = position();
    const Point dir = direction();

    drawBody(target, lines);

    if (trafficDebug) {
        // Target speed as a vector ahead of the car, coloured by what limits it.
        const sf::Color c = limiterColor(limiter_);
        const float len = 10.0f + targetSpeed_ * 0.5f;
        lines.draw(target, centre, {centre.x + dir.x * len, centre.y + dir.y * len}, c, 2.0f);
        if (limiter_ == SpeedLimiter::Signal || limiter_ == SpeedLimiter::Leader)
            lines.draw(target, centre, blockingPoint_, c, 1.0f);
    }

    if (!curveDebug) return;

    const auto pathPoints = path.sampledPoints(30);
    for (std::size_t i = 1; i < pathPoints.size(); ++i)
        lines.draw(target, pathPoints[i - 1], pathPoints[i], Colors::Debug, 1.0f);

    for (const auto& segment : path.segments()) {
        // Control polygon
        lines.draw(target, segment.p0, segment.p1, sf::Color(140, 140, 140), 1.0f);
        lines.draw(target, segment.p1, segment.p2, sf::Color(140, 140, 140), 1.0f);
        lines.draw(target, segment.p2, segment.p3, sf::Color(140, 140, 140), 1.0f);

        for (Point p : {segment.p0, segment.p1, segment.p2, segment.p3}) {
            ShapeAlgorithms::drawPoints(
                target, ShapeAlgorithms::filledCircle(p, 3),
                sf::Color(235, 190, 70), 1.0f);
        }
    }

    // Catmull-Rom knots (only for routes built from knots).
    for (Point knot : path.knots()) {
        ShapeAlgorithms::drawPoints(
            target, ShapeAlgorithms::filledCircle(knot, 6),
            sf::Color(220, 90, 200), 1.0f);
    }
}
