#include "TrafficSystem.h"
#include "TrafficRules.h"
#include "../core/Color.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace {

constexpr float Lookahead = 140.0f;   // how far ahead drivers look (px)

float cross(Point a, Point b) { return a.x * b.y - a.y * b.x; }

// Intersection parameter t on segment p->p2 with segment a->b, or -1.
float segmentHit(Point p, Point p2, Point a, Point b) {
    const Point r{p2.x - p.x, p2.y - p.y};
    const Point s{b.x - a.x, b.y - a.y};
    const float denom = cross(r, s);
    if (std::abs(denom) < 1e-6f) return -1.0f;
    const Point q{a.x - p.x, a.y - p.y};
    const float t = cross(q, s) / denom;
    const float u = cross(q, r) / denom;
    return (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) ? t : -1.0f;
}

}

TrafficSystem::TrafficSystem(const RoadNetwork& network,
                             int vehicleCount, unsigned seed)
    : network_(network), vehicleCount_(vehicleCount), seed_(seed) {
    // Matatu stages sit in the kerbside lane between junctions. In
    // left-hand traffic the kerb lane is the only lane per direction.
    stages_ = {
        {560, 165},   // top street, eastbound
        {640, 235},   // top street, westbound
        {470, 555},   // bottom street, westbound
        {835, 380},   // east avenue, southbound
    };

    // Waypoints: intersection centres and road ends just off the map.
    const Point A{300, 200}, B{800, 200}, C{300, 520}, D{800, 520};
    const Point north300{300, -60}, north800{800, -60}, south800{800, 780};
    const Point west200{-60, 200}, east200{1160, 200};
    const Point west520{-60, 520}, east520{1160, 520};

    const auto outerLefts  = makeRoute({C, D, B, A});
    const auto outerRights = makeRoute({A, B, D, C});
    const auto top         = makeRoute({west200, A, B, east200});
    const auto eastAvenue  = makeRoute({north800, B, D, south800});
    const auto westLoop    = makeRoute({north300, A, C, D, east520, D, C, A});
    const auto bottomLoop  = makeRoute({west520, C, D, B, east200, B, D, C});
    routes_ = {outerLefts, outerRights, top, eastAvenue, westLoop, bottomLoop};

    spawn(vehicleCount_, seed_);
}

void TrafficSystem::reset() {
    vehicles_.clear();
    spawn(vehicleCount_, seed_);
}

void TrafficSystem::spawn(int count, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    std::uniform_int_distribution<std::size_t> pickRoute(0, routes_.size() - 1);

    // Rejection sampling: retry positions that would overlap another
    // vehicle or start inside a junction box.
    auto clear = [&](Point p) {
        for (const auto& v : vehicles_) {
            const Point q = v.position();
            if (std::hypot(p.x - q.x, p.y - q.y) < 75.0f) return false;
        }
        for (const auto& node : network_.intersections()) {
            if (std::abs(p.x - node.centre.x) < RoadNetwork::StopLineDistance &&
                std::abs(p.y - node.centre.y) < RoadNetwork::StopLineDistance)
                return false;
        }
        return true;
    };

    int counters[3] = {0, 0, 0};
    for (int attempt = 0; attempt < count * 50 &&
                          static_cast<int>(vehicles_.size()) < count; ++attempt) {
        const auto& route = routes_[pickRoute(rng)];
        const float distance = unit(rng) * route->path.length();
        const Point p = route->path.pointAtDistance(distance);
        const bool onMap = p.x > 0 && p.x < RoadNetwork::WorldWidth &&
                           p.y > 0 && p.y < RoadNetwork::WorldHeight;
        if (!onMap || !clear(p)) continue;

        const VehicleType type = pickVehicleType(unit(rng));
        const VehicleSpec& spec = specFor(type);
        const float cruise = spec.minCruise + unit(rng) * (spec.maxCruise - spec.minCruise);
        const int palette = counters[static_cast<int>(type)]++;
        vehicles_.emplace_back(route, type, cruise, distance, palette);
    }
}

std::shared_ptr<const TrafficRoute>
TrafficSystem::makeRoute(const std::vector<Point>& waypoints) const {
    auto route = std::make_shared<TrafficRoute>();
    route->path = RoadNetwork::buildRoute(waypoints);

    // Walk the path in small steps and record where it crosses a stop line
    // in that line's direction of travel. Done once per route, so at run
    // time a vehicle only compares two arc lengths.
    const float length = route->path.length();
    const float step = 2.0f;
    const auto& stopLines = network_.stopLines();

    Point previous = route->path.pointAtDistance(0.0f);
    for (float s = step; s <= length; s += step) {
        const Point current = route->path.pointAtDistance(s);
        for (int i = 0; i < static_cast<int>(stopLines.size()); ++i) {
            const StopLine& line = stopLines[i];
            const float t = segmentHit(previous, current, line.a, line.b);
            if (t < 0.0f) continue;

            const Point d = headingVector(line.approach);
            const Point travel{current.x - previous.x, current.y - previous.y};
            if (travel.x * d.x + travel.y * d.y <= 0.0f) continue;

            route->stops.push_back({s - step + t * step, i});
        }

        // The route serves a stage when it passes over the stage point
        // travelling in the stage's lane.
        for (const Point& stage : stages_) {
            const float gap = std::hypot(current.x - stage.x, current.y - stage.y);
            if (gap < step * 0.75f) {
                const float last = route->stages.empty() ? -1e9f : route->stages.back();
                if (s - last > 20.0f) route->stages.push_back(s);
            }
        }
        previous = current;
    }
    return route;
}

void TrafficSystem::plan(const TrafficLights& lights,
                         const std::vector<std::vector<Point>>& obstacles) {
    const std::size_t n = vehicles_.size();

    // Pairwise forward-corridor checks from one snapshot of the traffic.
    std::vector<TrafficRules::CorridorHit> hits(n * n);
    std::vector<std::vector<Point>> footprints(n);
    for (std::size_t j = 0; j < n; ++j)
        footprints[j] = vehicles_[j].footprint();

    for (std::size_t i = 0; i < n; ++i) {
        const TrafficVehicle& v = vehicles_[i];
        const Point pos = v.position();
        const Point dir = v.direction();
        for (std::size_t j = 0; j < n; ++j) {
            if (i == j) continue;
            hits[i * n + j] = TrafficRules::corridorCheck(
                pos, dir, v.halfLength(), v.halfWidth(), Lookahead, footprints[j]);
        }
    }

    for (std::size_t i = 0; i < n; ++i) {
        TrafficVehicle& v = vehicles_[i];
        const float b = v.comfortableBraking();
        const float own = v.speed();
        const Point pos = v.position();
        const Point dir = v.direction();

        float target = v.cruiseSpeed();
        SpeedLimiter reason = SpeedLimiter::Free;
        Point blocking = pos;

        // 1. Slow down for corners: compare the heading now and a little ahead.
        const Point ahead = v.route().path.directionAtDistance(v.distance() + 40.0f);
        if (dir.x * ahead.x + dir.y * ahead.y < 0.95f) {
            target = v.cruiseSpeed() * 0.6f;
            reason = SpeedLimiter::Corner;
        }

        // 2. Nearest stop line ahead of the front bumper.
        const float length = v.route().path.length();
        const float front = v.distance() + v.halfLength();
        for (const auto& stop : v.route().stops) {
            float d = stop.distance - front;
            if (d < -1.0f) d += length;           // the route is a loop
            if (d < -1.0f || d > Lookahead) continue;

            const StopLine& line = network_.stopLines()[stop.stopLine];
            if (!TrafficRules::mustStop(lights.signalFor(line), d, own, b)) continue;

            const float limit = TrafficRules::stoppingSpeed(d - 2.0f, b);
            if (limit < target) {
                target = limit;
                reason = SpeedLimiter::Signal;
                blocking = {(line.a.x + line.b.x) * 0.5f, (line.a.y + line.b.y) * 0.5f};
            }
        }

        // 3. Matatus pull in at stages they have not just served.
        if (v.spec().stageDwell > 0.0f) {
            const auto& stages = v.route().stages;
            for (int k = 0; k < static_cast<int>(stages.size()); ++k) {
                if (k == v.servedStage()) continue;
                float d = stages[k] - v.distance();
                if (d < -1.0f) d += length;
                if (d < -1.0f || d > Lookahead) continue;
                const float limit = TrafficRules::stoppingSpeed(d, b);
                if (limit < target) {
                    target = limit;
                    reason = SpeedLimiter::Stage;
                    blocking = v.route().path.pointAtDistance(stages[k]);
                }
            }
        }

        // 4. Vehicles in the forward corridor. If two vehicles block each
        //    other (crossing paths), the one closer to the conflict goes.
        for (std::size_t j = 0; j < n; ++j) {
            if (i == j) continue;
            const auto& hit = hits[i * n + j];
            if (!hit.hit) continue;

            const auto& back = hits[j * n + i];
            const bool iHasPriority = back.hit &&
                (hit.gap < back.gap || (hit.gap == back.gap && i < j));
            if (iHasPriority) continue;

            const TrafficVehicle& other = vehicles_[j];
            const Point od = other.direction();
            const float leaderSpeed =
                std::max(0.0f, other.speed() * (od.x * dir.x + od.y * dir.y));
            const float limit = TrafficRules::followingSpeed(leaderSpeed, hit.gap, own, b);
            if (limit < target) {
                target = limit;
                reason = SpeedLimiter::Leader;
                blocking = hit.nearest;
            }
        }

        // 5. Other road users (the ambulance) are treated as stopped leaders.
        for (const auto& obstacle : obstacles) {
            const auto hit = TrafficRules::corridorCheck(
                pos, dir, v.halfLength(), v.halfWidth(), Lookahead, obstacle);
            if (!hit.hit) continue;
            const float limit = TrafficRules::followingSpeed(0.0f, hit.gap, own, b);
            if (limit < target) {
                target = limit;
                reason = SpeedLimiter::Leader;
                blocking = hit.nearest;
            }
        }

        v.setTarget(std::min(target, v.cruiseSpeed()), reason, blocking);
    }
}

void TrafficSystem::update(float deltaTime,
                           const TrafficLights& lights,
                           const std::vector<std::vector<Point>>& obstacles) {
    plan(lights, obstacles);
    for (auto& vehicle : vehicles_)
        vehicle.integrate(deltaTime);
}

void TrafficSystem::drawStages(sf::RenderTarget& target,
                               const LineRenderer& lines) const {
    const sf::Color paint(235, 200, 60);
    for (const Point& stage : stages_) {
        // Bay outline in the lane, aligned with the road it sits on.
        const bool vertical = std::abs(stage.x - 300.0f) < RoadNetwork::HalfWidth ||
                              std::abs(stage.x - 800.0f) < RoadNetwork::HalfWidth;
        const float hx = vertical ? 13.0f : 38.0f;
        const float hy = vertical ? 38.0f : 13.0f;
        const Point c[] = {{stage.x - hx, stage.y - hy}, {stage.x + hx, stage.y - hy},
                           {stage.x + hx, stage.y + hy}, {stage.x - hx, stage.y + hy}};
        for (int i = 0; i < 4; ++i)
            lines.draw(target, c[i], c[(i + 1) % 4], paint, 1.0f);
        lines.draw(target, c[0], c[2], paint, 1.0f);
        lines.draw(target, c[1], c[3], paint, 1.0f);
    }
}

void TrafficSystem::draw(sf::RenderTarget& target,
                         const LineRenderer& lines,
                         bool curveDebug,
                         bool trafficDebug) const {
    drawStages(target, lines);
    for (const auto& vehicle : vehicles_)
        vehicle.draw(target, lines, curveDebug, trafficDebug);
}

const std::vector<TrafficVehicle>& TrafficSystem::vehicles() const {
    return vehicles_;
}

const std::vector<Point>& TrafficSystem::stages() const {
    return stages_;
}
