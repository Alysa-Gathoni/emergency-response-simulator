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
                             TrafficDensity density, unsigned seed)
    : network_(network), density_(density), seed_(seed) {
    placeStages();
    buildRoutes();
    spawn(vehicleCountFor(density_, static_cast<int>(network_.intersections().size())), seed_);
}

int TrafficSystem::vehicleCountFor(TrafficDensity density, int intersections) {
    const float perJunction = density == TrafficDensity::Low    ? 2.5f
                            : density == TrafficDensity::Medium ? 3.5f : 5.0f;
    return std::max(4, static_cast<int>(std::lround(perJunction * intersections)));
}

void TrafficSystem::setDensity(TrafficDensity density) {
    density_ = density;
    reset();
}

TrafficDensity TrafficSystem::density() const { return density_; }
int TrafficSystem::routeCount() const { return static_cast<int>(routes_.size()); }

void TrafficSystem::placeStages() {
    // A matatu stage in the middle of every road section between two
    // junctions, alternating the direction (lane) it serves.
    stages_.clear();
    int k = 0;
    for (const Road& road : network_.roads()) {
        const float extent = road.vertical ? network_.height() : network_.width();
        for (const auto& [a, b] : network_.freeIntervals(road)) {
            if (a <= 0.0f || b >= extent || b - a < 160.0f) continue;   // map edge sections
            const float mid = (a + b) * 0.5f;
            const Point centre = road.vertical ? Point{road.centre, mid} : Point{mid, road.centre};
            const float sign = (k++ % 2 == 0) ? 1.0f : -1.0f;
            const Point travel = road.vertical ? Point{0.0f, sign} : Point{sign, 0.0f};
            stages_.push_back(RoadNetwork::laneCentre(centre, travel));
        }
    }
}

void TrafficSystem::buildRoutes() {
    // Every route is a closed list of grid waypoints (intersection centres
    // or road ends just off the map); RoadNetwork::buildRoute turns it into
    // lane-correct straights, Bezier corners and U-turns at road ends.
    const auto xs = network_.columns();
    const auto ys = network_.rows();
    const int nx = static_cast<int>(xs.size());
    const int ny = static_cast<int>(ys.size());
    const float off = 60.0f;
    const float right = network_.width() + off;
    const float bottom = network_.height() + off;
    auto node = [&](int i, int j) { return Point{xs[i], ys[j]}; };

    std::vector<std::vector<Point>> plans;

    // 1. Round every grid cell, both ways (all-left and all-right turns).
    for (int i = 0; i + 1 < nx; ++i) {
        for (int j = 0; j + 1 < ny; ++j) {
            const Point a = node(i, j), b = node(i + 1, j);
            const Point c = node(i + 1, j + 1), d = node(i, j + 1);
            plans.push_back({a, b, c, d});
            plans.push_back({d, c, b, a});
        }
    }

    // 2. Round the whole grid, when it has more than one cell.
    if (nx > 2 || ny > 2) {
        const Point a = node(0, 0), b = node(nx - 1, 0);
        const Point c = node(nx - 1, ny - 1), d = node(0, ny - 1);
        plans.push_back({a, b, c, d});
        plans.push_back({d, c, b, a});
    }

    // 3. Straight along every road, end to end (U-turns off the map).
    for (float y : ys) plans.push_back({{-off, y}, {right, y}});
    for (float x : xs) plans.push_back({{x, -off}, {x, bottom}});

    // 4. Zig-zags between neighbouring roads: in from the map edge, turn at
    //    the far junction, back along the next road.
    for (int j = 0; j + 1 < ny; ++j)
        plans.push_back({{-off, ys[j]}, node(nx - 1, j), node(nx - 1, j + 1), {-off, ys[j + 1]}});
    for (int i = 0; i + 1 < nx; ++i)
        plans.push_back({{xs[i], -off}, node(i, ny - 1), node(i + 1, ny - 1), {xs[i + 1], -off}});

    routes_.clear();
    for (const auto& waypoints : plans)
        routes_.push_back(makeRoute(waypoints));
}

void TrafficSystem::keepClear(Point p, float radius) {
    keepClear_.push_back({p, radius});
}

int TrafficSystem::yieldingCount() const {
    int n = 0;
    for (const auto& v : vehicles_) n += v.pullingOver() ? 1 : 0;
    return n;
}

void TrafficSystem::reset() {
    vehicles_.clear();
    spawn(vehicleCountFor(density_, static_cast<int>(network_.intersections().size())), seed_);
}

void TrafficSystem::spawn(int count, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    std::uniform_int_distribution<std::size_t> pickRoute(0, routes_.size() - 1);

    // Rejection sampling: retry positions that would overlap another
    // vehicle or start inside a junction box.
    auto clear = [&](Point p) {
        for (const auto& [centre, radius] : keepClear_)
            if (std::hypot(p.x - centre.x, p.y - centre.y) < radius) return false;
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
        const bool onMap = p.x > 0 && p.x < network_.width() &&
                           p.y > 0 && p.y < network_.height();
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
    int inside = network_.junctionAt(previous);
    float enteredAt = 0.0f;
    for (float s = step; s <= length; s += step) {
        const Point current = route->path.pointAtDistance(s);

        // Entering / leaving an intersection box.
        const int box = network_.junctionAt(current);
        if (box != inside) {
            if (inside >= 0) {
                const Point in = route->path.directionAtDistance(enteredAt);
                const Point out = route->path.directionAtDistance(s);
                route->junctions.push_back({inside, enteredAt, s,
                                            in.x * out.x + in.y * out.y < 0.7f});
            }
            if (box >= 0) enteredAt = s;
            inside = box;
        }
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
    if (inside >= 0) {
        const Point in = route->path.directionAtDistance(enteredAt);
        const Point out = route->path.directionAtDistance(length);
        route->junctions.push_back({inside, enteredAt, length, in.x * out.x + in.y * out.y < 0.7f});
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

    // Junction boxes: who is inside (or about to enter) which box.
    struct BoxState {
        int junction{-1};
        bool inside{false};
        bool turns{false};
        float toEnter{0.0f};    // distance from the front bumper to the box
    };
    std::vector<BoxState> boxes(n);
    for (std::size_t i = 0; i < n; ++i) {
        const TrafficVehicle& v = vehicles_[i];
        const float length = v.route().path.length();
        const float s = v.distance();
        float best = 1e9f;
        for (const auto& pass : v.route().junctions) {
            if (s + v.halfLength() >= pass.enter && s - v.halfLength() <= pass.exit) {
                boxes[i] = {pass.junction, true, pass.turns, 0.0f};
                break;
            }
            float d = pass.enter - (s + v.halfLength());
            if (d < 0.0f) d += length;
            if (d < best) {
                best = d;
                boxes[i] = {pass.junction, false, pass.turns, d};
            }
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

        // 4. Yield at the junction box: a vehicle about to enter waits while
        //    a conflicting vehicle is inside (or committed just ahead of it).
        //    Conflicting = different direction, except two vehicles going
        //    straight past each other in opposite lanes.
        const BoxState& mine = boxes[i];
        if (mine.junction >= 0 && !mine.inside && mine.toEnter < Lookahead) {
            for (std::size_t j = 0; j < n; ++j) {
                if (i == j || boxes[j].junction != mine.junction) continue;
                const BoxState& other = boxes[j];
                const bool committed = !other.inside && other.toEnter < 4.0f &&
                                       vehicles_[j].speed() > 15.0f &&
                                       (other.toEnter < mine.toEnter ||
                                        (other.toEnter == mine.toEnter && j < i));
                if (!other.inside && !committed) continue;

                const Point od = vehicles_[j].direction();
                const float dot = od.x * dir.x + od.y * dir.y;
                const bool sameWay = dot > 0.9f;
                const bool oppositeStraight = dot < -0.9f && !mine.turns && !other.turns;
                if (sameWay || oppositeStraight) continue;

                const float limit = TrafficRules::stoppingSpeed(std::max(0.0f, mine.toEnter - 2.0f), b);
                if (limit < target) {
                    target = limit;
                    reason = SpeedLimiter::Yield;
                    blocking = vehicles_[j].position();
                }
            }
        }

        // 5. Vehicles in the forward corridor. If two vehicles block each
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

        // 6. Other road users (the ambulance) are treated as stopped leaders.
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

        // 7. Siren: make way for the ambulance.
        bool pullOver = false;
        if (emergency_.siren) {
            const Point e = emergency_.position;
            const Point ed = emergency_.direction;
            const Point rel{pos.x - e.x, pos.y - e.y};
            const float along = rel.x * ed.x + rel.y * ed.y;          // ahead of the ambulance
            const float side = std::abs(rel.x * -ed.y + rel.y * ed.x);
            const float facing = dir.x * ed.x + dir.y * ed.y;
            const bool onItsRoad = side < RoadNetwork::HalfWidth + 10.0f;

            if (onItsRoad && along > -20.0f && along < SirenReach && facing > 0.7f) {
                // Same direction, in front: pull over and crawl.
                pullOver = true;
                if (SirenCrawl < target) {
                    target = SirenCrawl;
                    reason = SpeedLimiter::Siren;
                    blocking = e;
                }
            } else if (onItsRoad && along > 0.0f && along < SirenReach && facing < -0.7f) {
                // Oncoming: slow down and keep left.
                pullOver = true;
                if (v.cruiseSpeed() * 0.5f < target) {
                    target = v.cruiseSpeed() * 0.5f;
                    reason = SpeedLimiter::Siren;
                    blocking = e;
                }
            }

            // Crossing traffic holds before a junction the ambulance is about
            // to use (or is inside).
            const BoxState& box = boxes[i];
            if (!pullOver && box.junction >= 0 && !box.inside && box.toEnter < Lookahead) {
                const Point c = network_.intersections()[static_cast<std::size_t>(box.junction)].centre;
                const Point toBox{c.x - e.x, c.y - e.y};
                const float ahead = toBox.x * ed.x + toBox.y * ed.y;
                const float dist = std::hypot(toBox.x, toBox.y);
                if (dist < 60.0f || (ahead > -40.0f && dist < 240.0f && std::abs(facing) < 0.7f)) {
                    const float limit = TrafficRules::stoppingSpeed(std::max(0.0f, box.toEnter - 2.0f), b);
                    if (limit < target) {
                        target = limit;
                        reason = SpeedLimiter::Siren;
                        blocking = c;
                    }
                }
            }
        }
        v.setPullOver(pullOver);

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

void TrafficSystem::drawStages(IRenderer& target,
                               const LineRenderer& lines) const {
    const Rgba paint(235, 200, 60);
    for (const Point& stage : stages_) {
        // Bay outline in the lane, aligned with the road it sits on.
        bool vertical = false;
        for (const Road& road : network_.roads())
            if (road.vertical && std::abs(stage.x - road.centre) < RoadNetwork::HalfWidth)
                vertical = true;
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

void TrafficSystem::draw(IRenderer& target,
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
