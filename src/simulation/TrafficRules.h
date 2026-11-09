#pragma once
#include <vector>
#include "../core/Point.h"
#include "../city/SignalController.h"

// Pure maths behind civilian driving behaviour (no rendering, testable).
//
// Everything is built on the constant-deceleration stopping relation
//     v^2 = 2 * b * d
// A vehicle with speed v needs distance v^2 / (2b) to stop with braking b,
// so the fastest speed from which it can still stop within d is sqrt(2bd).
namespace TrafficRules {

// Highest speed that still allows stopping within `distance`.
float stoppingSpeed(float distance, float braking);

// Braking distance from `speed`.
float brakingDistance(float speed, float braking);

// Red: always stop. Yellow: stop only if a comfortable stop is possible,
// otherwise continue (stopping hard inside the "dilemma zone" is worse).
// Green: go.
bool mustStop(Signal signal, float distanceToLine, float speed, float braking);

// Speed that keeps a safe gap behind a leader moving at leaderSpeed:
// even if the leader brakes at the same rate, we stop before reaching it.
// A standstill gap and a short time headway are kept free.
float followingSpeed(float leaderSpeed, float gap, float ownSpeed, float braking);

// Is anything from `footprint` in the forward corridor of a vehicle at
// `position` heading `direction`? The footprint is projected into the
// vehicle's local frame (along, lateral); on a hit `gap` is the free
// distance between our front bumper and the nearest part of the obstacle.
struct CorridorHit {
    bool hit{false};
    float gap{0.0f};
    Point nearest{};
};

CorridorHit corridorCheck(Point position, Point direction,
                          float halfLength, float halfWidth,
                          float lookahead,
                          const std::vector<Point>& footprint);

}
