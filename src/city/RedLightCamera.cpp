#include "RedLightCamera.h"
#include "../shapes/CircleEllipse.h"
#include <algorithm>
#include <cmath>

namespace {

float cross(Point a, Point b) { return a.x * b.y - a.y * b.x; }

// True when segment p->q crosses segment a->b (parametric form:
// p + t(q - p) = a + u(b - a) with t, u in [0, 1]).
bool crosses(Point p, Point q, Point a, Point b) {
    const Point r{q.x - p.x, q.y - p.y};
    const Point s{b.x - a.x, b.y - a.y};
    const float denom = cross(r, s);
    if (std::abs(denom) < 1e-6f) return false;
    const Point w{a.x - p.x, a.y - p.y};
    const float t = cross(w, s) / denom;
    const float u = cross(w, r) / denom;
    return t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f;
}

}

RedLightCamera::RedLightCamera(const RoadNetwork& network,
                               const TrafficLights& lights)
    : network_(network), lights_(lights) {}

bool RedLightCamera::update(Point previous, Point current, float deltaTime) {
    flash_ = std::max(0.0f, flash_ - deltaTime);

    const Point motion{current.x - previous.x, current.y - previous.y};
    for (const StopLine& line : network_.stopLines()) {
        // Only crossing in the approach direction counts (leaving the
        // junction over the opposite lane's line is not a violation).
        const Point d = headingVector(line.approach);
        if (motion.x * d.x + motion.y * d.y <= 0.0f) continue;
        if (!crosses(previous, current, line.a, line.b)) continue;
        if (lights_.signalFor(line) != Signal::Red) continue;

        ++violations_;
        flash_ = 0.6f;
        flashAt_ = {(line.a.x + line.b.x) * 0.5f, (line.a.y + line.b.y) * 0.5f};
        return true;
    }
    return false;
}

int RedLightCamera::violations() const { return violations_; }

void RedLightCamera::reset() {
    violations_ = 0;
    flash_ = 0.0f;
}

void RedLightCamera::draw(sf::RenderTarget& target, const LineRenderer&) const {
    if (flash_ <= 0.0f) return;
    // Camera flash: an expanding midpoint circle that fades out.
    const float t = 1.0f - flash_ / 0.6f;
    const int radius = 10 + static_cast<int>(t * 40.0f);
    const auto alpha = static_cast<sf::Uint8>(255.0f * (1.0f - t));
    ShapeAlgorithms::drawPoints(target,
                                ShapeAlgorithms::midpointCircle(flashAt_, radius),
                                sf::Color(255, 255, 255, alpha), 3.0f);
}
