#include "TrafficVehicle.h"
#include "VehicleModels.h"
#include "../core/Mat3.h"
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

Rgba limiterColor(SpeedLimiter reason) {
    switch (reason) {
        case SpeedLimiter::Free:   return Rgba(80, 220, 170);
        case SpeedLimiter::Corner: return Rgba(90, 160, 240);
        case SpeedLimiter::Signal: return Rgba(235, 70, 70);
        case SpeedLimiter::Leader: return Rgba(240, 170, 60);
        case SpeedLimiter::Stage:  return Rgba(220, 90, 200);
    }
    return Rgba::White;
}

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
    age_ += deltaTime;
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

bool TrafficVehicle::braking() const {
    return speed_ > 0.5f && targetSpeed_ < speed_ - 2.0f;
}

int TrafficVehicle::indicator() const {
    // Compare the heading now with the heading 70 px further along the
    // route. The z component of their cross product gives the turn side:
    // positive = clockwise on screen = right turn (y points down).
    const Point now = direction();
    const Point later = route_->path.directionAtDistance(distance_ + 70.0f);
    const float turn = now.x * later.y - now.y * later.x;
    if (turn > 0.3f) return 1;
    if (turn < -0.3f) return -1;
    return 0;
}

void TrafficVehicle::drawBody(IRenderer& target,
                              const LineRenderer& lines) const {
    const Point c = position();
    const Point d = direction();
    // One composite matrix places the whole model: T(position) * R(heading).
    const Mat3 world = Mat3::translation(c.x, c.y) *
                       Mat3::rotation(std::atan2(d.y, d.x));
    const VehiclePose pose{world, spec_->type, paletteIndex_, braking(),
                           indicator(), std::fmod(age_, 0.8f) < 0.4f};
    VehicleModels::draw(target, lines, pose);
}

void TrafficVehicle::draw(IRenderer& target,
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
        const Rgba c = limiterColor(limiter_);
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
        lines.draw(target, segment.p0, segment.p1, Rgba(140, 140, 140), 1.0f);
        lines.draw(target, segment.p1, segment.p2, Rgba(140, 140, 140), 1.0f);
        lines.draw(target, segment.p2, segment.p3, Rgba(140, 140, 140), 1.0f);

        for (Point p : {segment.p0, segment.p1, segment.p2, segment.p3}) {
            ShapeAlgorithms::drawPoints(
                target, ShapeAlgorithms::filledCircle(p, 3),
                Rgba(235, 190, 70), 1.0f);
        }
    }

    // Catmull-Rom knots (only for routes built from knots).
    for (Point knot : path.knots()) {
        ShapeAlgorithms::drawPoints(
            target, ShapeAlgorithms::filledCircle(knot, 6),
            Rgba(220, 90, 200), 1.0f);
    }
}
