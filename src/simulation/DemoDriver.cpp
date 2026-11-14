#include "DemoDriver.h"
#include "../city/RoadNetwork.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float PI = 3.14159265358979323846f;

float length(Point a, Point b) { return std::hypot(b.x - a.x, b.y - a.y); }

float wrap(float radians) {
    while (radians > PI) radians -= 2.0f * PI;
    while (radians < -PI) radians += 2.0f * PI;
    return radians;
}
}

Point DemoDriver::along(const std::vector<Point>& route, float distance, Point* direction) {
    if (route.empty()) return {0, 0};
    if (direction) *direction = {1, 0};
    float left = std::max(0.0f, distance);
    for (std::size_t i = 1; i < route.size(); ++i) {
        const float len = length(route[i - 1], route[i]);
        if (len <= 0.0f) continue;
        const Point d{(route[i].x - route[i - 1].x) / len, (route[i].y - route[i - 1].y) / len};
        if (direction) *direction = d;
        if (left <= len) return {route[i - 1].x + d.x * left, route[i - 1].y + d.y * left};
        left -= len;
    }
    return route.back();
}

DrivingInput DemoDriver::drive(const Ambulance& ambulance, const std::vector<Point>& route,
                               const std::vector<Point>& obstacles, bool stopAtEnd,
                               float deltaTime) {
    DrivingInput in;
    if (route.size() < 2) {
        in.throttle = ambulance.speed() > 1.0f ? -1.0f : 0.0f;
        return in;
    }

    float total = 0.0f;
    for (std::size_t i = 1; i < route.size(); ++i) total += length(route[i - 1], route[i]);

    const Point p = ambulance.position();
    const float speed = ambulance.speed();
    const float heading = ambulance.heading() * PI / 180.0f;
    const Point fwd{std::cos(heading), std::sin(heading)};
    const Point end = route.back();
    const float toEnd = length(p, end);

    // Overshot a nearby target (it is behind): reverse straight back.
    const float endAhead = (end.x - p.x) * fwd.x + (end.y - p.y) * fwd.y;
    if (stopAtEnd && toEnd < 140.0f && endAhead < -12.0f) {
        in.throttle = speed > 0.0f ? -1.0f : (toEnd > 20.0f ? -0.7f : 0.0f);
        return in;
    }

    // Pure pursuit on the lane beside the route's centre line; the last
    // stretch aims at the target itself (it already is in a lane).
    const float lookahead = 45.0f + std::abs(speed) * 0.35f;
    Point dir;
    Point aim = along(route, std::min(lookahead, total), &dir);
    if (total - lookahead > 60.0f) aim = RoadNetwork::laneCentre(aim, dir);
    else aim = end;
    const float error = wrap(std::atan2(aim.y - p.y, aim.x - p.x) - heading);
    in.steer = std::clamp(error * 2.6f, -1.0f, 1.0f);

    // Target speed: stop on the target (straight-line distance, the route
    // via the centre line is longer than the lane), slow for a bend ahead,
    // slow while the nose still points the wrong way.
    float target = CruiseSpeed;
    if (stopAtEnd) {
        const float d = std::min(total, toEnd);
        target = d < 14.0f ? 0.0f : std::min(target, std::sqrt(2.0f * Braking * (d - 14.0f)));
    }
    Point first, ahead;
    along(route, 45.0f, &first);
    for (float s = 65.0f; s < std::min(total, 65.0f + std::abs(speed) * 1.1f); s += 20.0f) {
        along(route, s, &ahead);
        if (first.x * ahead.x + first.y * ahead.y < 0.7f) { target = std::min(target, TurnSpeed); break; }
    }
    if (std::abs(error) > 0.6f) target = std::min(target, TurnSpeed * 0.8f);

    // Anything in the lane just ahead: brake. Two vehicles can each wait
    // for the other (traffic gives way to the ambulance), so after a few
    // seconds of waiting the driver creeps on regardless.
    bool blocked = false;
    for (const Point& o : obstacles) {
        const Point d{o.x - p.x, o.y - p.y};
        const float front = d.x * fwd.x + d.y * fwd.y;
        const float side = std::abs(d.x * -fwd.y + d.y * fwd.x);
        if (front > 20.0f && front < 55.0f + std::abs(speed) * 0.45f && side < 22.0f) blocked = true;
    }
    waited_ = blocked ? waited_ + deltaTime : 0.0f;
    if (blocked && waited_ < 3.0f) target = 0.0f;
    else if (blocked) target = std::min(target, 25.0f);

    in.throttle = std::clamp((target - speed) / 40.0f, -1.0f, 1.0f);
    if (target == 0.0f && speed < 2.0f) in.throttle = 0.0f;

    // Three-point turn: the turning circle (wheelbase / tan(max steer),
    // about 62 px) does not fit a U-turn between the kerbs, so when the
    // ambulance stalls against something while trying to go forward it
    // backs up with the opposite lock for a moment, then tries again.
    if (reversing_ > 0.0f) {
        reversing_ -= deltaTime;
        in.throttle = -1.0f;
        in.steer = -reverseSteer_;
        return in;
    }
    stalled_ = in.throttle > 0.3f && std::abs(speed) < 4.0f ? stalled_ + deltaTime : 0.0f;
    if (stalled_ > 0.5f) {
        stalled_ = 0.0f;
        reversing_ = 1.0f;
        reverseSteer_ = in.steer >= 0.0f ? 1.0f : -1.0f;
    }
    return in;
}
