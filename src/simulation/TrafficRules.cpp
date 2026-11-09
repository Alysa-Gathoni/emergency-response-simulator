#include "TrafficRules.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace TrafficRules {

namespace {
constexpr float StandstillGap = 7.0f;   // pixels left free when queued
constexpr float TimeHeadway = 0.4f;     // seconds of extra gap per unit speed
constexpr float CorridorMargin = 3.0f;  // extra width each side of the car
}

float stoppingSpeed(float distance, float braking) {
    return std::sqrt(2.0f * braking * std::max(0.0f, distance));
}

float brakingDistance(float speed, float braking) {
    return braking > 0.0f ? speed * speed / (2.0f * braking) : 0.0f;
}

bool mustStop(Signal signal, float distanceToLine, float speed, float braking) {
    switch (signal) {
        case Signal::Red:    return true;
        case Signal::Yellow: return brakingDistance(speed, braking) <= distanceToLine;
        case Signal::Green:  return false;
    }
    return true;
}

float followingSpeed(float leaderSpeed, float gap, float ownSpeed, float braking) {
    // Both vehicles brake at b: the leader stops within vL^2 / 2b, so the
    // follower may use that distance as well as the current free gap.
    const float usable = gap - StandstillGap - TimeHeadway * ownSpeed;
    const float squared = leaderSpeed * leaderSpeed + 2.0f * braking * usable;
    return squared > 0.0f ? std::sqrt(squared) : 0.0f;
}

CorridorHit corridorCheck(Point position, Point direction,
                          float halfLength, float halfWidth,
                          float lookahead,
                          const std::vector<Point>& footprint) {
    CorridorHit result;
    if (footprint.empty()) return result;

    // Project every corner into the local frame (along, lateral). The
    // obstacle's extent in that frame is a conservative box around it.
    float aMin = std::numeric_limits<float>::max();
    float aMax = std::numeric_limits<float>::lowest();
    float lMin = aMin;
    float lMax = aMax;
    for (const Point& p : footprint) {
        const float rx = p.x - position.x;
        const float ry = p.y - position.y;
        const float along = rx * direction.x + ry * direction.y;
        const float lateral = -rx * direction.y + ry * direction.x;
        aMin = std::min(aMin, along);
        aMax = std::max(aMax, along);
        lMin = std::min(lMin, lateral);
        lMax = std::max(lMax, lateral);
    }

    const float w = halfWidth + CorridorMargin;
    const bool overlapsSideways = lMin < w && lMax > -w;
    const bool inFront = aMax > halfLength && aMin < halfLength + lookahead;
    if (!overlapsSideways || !inFront) return result;

    result.hit = true;
    result.gap = std::max(0.0f, aMin - halfLength);
    const float along = std::max(aMin, halfLength);
    result.nearest = {position.x + direction.x * along,
                      position.y + direction.y * along};
    return result;
}

}
