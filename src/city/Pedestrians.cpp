#include "Pedestrians.h"
#include "../shapes/CircleEllipse.h"
#include "../core/Color.h"
#include <algorithm>
#include <cmath>

namespace {

constexpr float CrossingLength = 2.0f * (RoadNetwork::HalfWidth + 10.0f);
constexpr int MaxWaitingPerKerb = 3;

const Rgba Clothes[] = {
    {200, 70, 70}, {70, 120, 200}, {230, 180, 60}, {90, 170, 110},
    {170, 90, 180}, {225, 225, 225}
};
const Rgba Skin[] = {{92, 60, 40}, {120, 80, 52}, {70, 46, 32}};

SignalGroup other(SignalGroup g) {
    return g == SignalGroup::NorthSouth ? SignalGroup::EastWest : SignalGroup::NorthSouth;
}

}

Pedestrians::Pedestrians(const RoadNetwork& network, unsigned seed)
    : network_(network), rng_(seed) {
    std::uniform_real_distribution<float> first(0.5f, 6.0f);
    spawnTimers_.resize(network.crosswalks().size() * 2);
    for (float& t : spawnTimers_) t = first(rng_);
}

bool Pedestrians::walkAllowed(const Crosswalk& cw, const TrafficLights& lights) const {
    // Walk while the vehicles that drive over this crossing are held at red
    // and the other group has green (never during all-red or yellow).
    return lights.signalAt(cw.intersection, cw.trafficGroup) == Signal::Red &&
           lights.signalAt(cw.intersection, other(cw.trafficGroup)) == Signal::Green;
}

Point Pedestrians::positionOf(const Walker& w) const {
    const Crosswalk& cw = network_.crosswalks()[w.crosswalk];
    const Point a = w.forward ? cw.start : cw.end;
    const Point b = w.forward ? cw.end : cw.start;
    const float t = w.crossing ? w.progress : 0.0f;
    // People waiting at the kerb stand in a small loose group on the
    // pavement, spread along the arm so they do not overlap.
    const Point d = directionOf(w);
    if (!w.crossing) {
        const Point arm = headingVector(cw.arm);
        const float spread = static_cast<float>(w.palette % 3 - 1) * 6.0f;
        return {a.x - d.x * 2.0f + arm.x * spread, a.y - d.y * 2.0f + arm.y * spread};
    }
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

Point Pedestrians::directionOf(const Walker& w) const {
    const Crosswalk& cw = network_.crosswalks()[w.crosswalk];
    const float dx = cw.end.x - cw.start.x;
    const float dy = cw.end.y - cw.start.y;
    const float len = std::hypot(dx, dy);
    const float s = w.forward ? 1.0f : -1.0f;
    return {s * dx / len, s * dy / len};
}

void Pedestrians::update(float deltaTime, const TrafficLights& lights) {
    std::uniform_real_distribution<float> interval(3.0f, 9.0f);
    std::uniform_real_distribution<float> pace(24.0f, 34.0f);
    std::uniform_int_distribution<int> colour(0, 63);

    const auto& crosswalks = network_.crosswalks();

    // New arrivals at each kerb.
    for (std::size_t i = 0; i < spawnTimers_.size(); ++i) {
        spawnTimers_[i] -= deltaTime;
        if (spawnTimers_[i] > 0.0f) continue;
        spawnTimers_[i] = interval(rng_);

        const int cw = static_cast<int>(i / 2);
        const bool forward = (i % 2) == 0;
        const int waiting = static_cast<int>(std::count_if(
            walkers_.begin(), walkers_.end(), [&](const Walker& w) {
                return w.crosswalk == cw && w.forward == forward && !w.crossing;
            }));
        if (waiting >= MaxWaitingPerKerb) continue;
        walkers_.push_back({cw, forward, false, 0.0f, 0.0f, pace(rng_), colour(rng_)});
    }

    for (Walker& w : walkers_) {
        const Crosswalk& cw = crosswalks[w.crosswalk];
        if (!w.crossing) {
            // Only step off if the walk phase lasts long enough to finish.
            const float remaining =
                lights.controller(cw.intersection).timeUntilChange();
            if (walkAllowed(cw, lights) && remaining * w.speed > CrossingLength)
                w.crossing = true;
            continue;
        }
        w.progress += w.speed * deltaTime / CrossingLength;
        w.stepPhase += w.speed * deltaTime * 0.35f;
    }

    walkers_.erase(std::remove_if(walkers_.begin(), walkers_.end(),
                                  [](const Walker& w) { return w.progress >= 1.0f; }),
                   walkers_.end());
}

std::vector<std::vector<Point>> Pedestrians::footprints() const {
    std::vector<std::vector<Point>> result;
    for (const Walker& w : walkers_) {
        if (!w.crossing) continue;
        const Point p = positionOf(w);
        result.push_back({{p.x - 6, p.y - 6}, {p.x + 6, p.y - 6},
                          {p.x + 6, p.y + 6}, {p.x - 6, p.y + 6}});
    }
    return result;
}

void Pedestrians::draw(IRenderer& target, const LineRenderer& lines,
                       const TrafficLights& lights, bool debug) const {
    // Pedestrian signal lamps at both ends of every crossing.
    for (const Crosswalk& cw : network_.crosswalks()) {
        const Rgba lamp = walkAllowed(cw, lights) ? Rgba(90, 230, 120)
                                                       : Rgba(230, 70, 60);
        // Lamps stand on the corner, just outside the zebra stripes.
        const Point arm = headingVector(cw.arm);
        for (Point kerb : {cw.start, cw.end}) {
            const Point end{kerb.x + arm.x * 13.0f, kerb.y + arm.y * 13.0f};
            ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(end, 3),
                                        Rgba(30, 30, 32), 1.0f);
            ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(end, 2),
                                        lamp, 1.0f);
        }
    }

    for (const Walker& w : walkers_) {
        const Point p = positionOf(w);
        const Point d = directionOf(w);
        const bool alongX = std::abs(d.x) > std::abs(d.y);

        // Legs: two small discs swinging forwards and back (sin of the
        // step phase), drawn under the body.
        const float swing = w.crossing ? std::sin(w.stepPhase) * 3.0f : 0.0f;
        const Point side = {-d.y * 2.0f, d.x * 2.0f};
        for (float s : {1.0f, -1.0f}) {
            const Point foot{p.x + side.x * s + d.x * swing * s,
                             p.y + side.y * s + d.y * swing * s};
            ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(foot, 1),
                                        Rgba(30, 30, 30), 1.0f);
        }

        // Shoulders: a midpoint ellipse wide across the walking direction.
        const int rx = alongX ? 2 : 5;
        const int ry = alongX ? 5 : 2;
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledEllipse(p, rx, ry),
                                    Clothes[w.palette % 6], 1.0f);
        // Head seen from above.
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(p, 2),
                                    Skin[w.palette % 3], 1.0f);

        if (debug) {
            const Point ahead{p.x + d.x * 12.0f, p.y + d.y * 12.0f};
            lines.draw(target, p, ahead, Colors::Debug, 1.0f);
        }
    }
}
