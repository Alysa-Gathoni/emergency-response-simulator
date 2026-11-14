#include "RouteGuide.h"
#include "../core/Color.h"
#include "../rasterization/PolygonFill.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace {

constexpr float DashLength = 14.0f;
constexpr float DashGap = 10.0f;
constexpr float FlowSpeed = 40.0f;      // px/s the dashes move towards the incident
constexpr float ArrowDistance = 50.0f;  // from the ambulance centre
const Rgba Outline(15, 18, 24);    // dark edge so the route stands out
                                        // from the painted road markings
constexpr float MergeDistance = 2.0f;   // nodes closer than this are one node
constexpr float StartGap = 36.0f;       // no dashes under the ambulance itself

float dist(Point a, Point b) { return std::hypot(b.x - a.x, b.y - a.y); }

// Nearest point on any road centre line, and which road it is on.
Point joinRoad(const RoadNetwork& network, Point p, int& roadIndex) {
    float best = std::numeric_limits<float>::max();
    Point join = p;
    const auto& roads = network.roads();
    for (int i = 0; i < static_cast<int>(roads.size()); ++i) {
        const Road& r = roads[i];
        const float d = r.vertical ? std::abs(p.x - r.centre) : std::abs(p.y - r.centre);
        if (d < best) {
            best = d;
            roadIndex = i;
            join = r.vertical ? Point{r.centre, p.y} : Point{p.x, r.centre};
        }
    }
    return join;
}

bool onRoad(const Road& r, Point p) {
    return r.vertical ? std::abs(p.x - r.centre) < MergeDistance
                      : std::abs(p.y - r.centre) < MergeDistance;
}

}

std::vector<Point> RouteGuide::plan(const RoadNetwork& network, Point from, Point to) {
    int startRoad = 0, endRoad = 0;
    const Point start = joinRoad(network, from, startRoad);
    const Point end = joinRoad(network, to, endRoad);

    // Nodes: 0 = start join, 1 = end join, then the intersections.
    std::vector<Point> nodes = {start, end};
    for (const Intersection& i : network.intersections())
        nodes.push_back(i.centre);

    // Edges between neighbouring nodes along every road.
    const int n = static_cast<int>(nodes.size());
    std::vector<std::vector<std::pair<int, float>>> edges(n);
    for (const Road& road : network.roads()) {
        std::vector<int> along;
        for (int i = 0; i < n; ++i)
            if (onRoad(road, nodes[i])) along.push_back(i);
        std::sort(along.begin(), along.end(), [&](int a, int b) {
            return road.vertical ? nodes[a].y < nodes[b].y : nodes[a].x < nodes[b].x;
        });
        for (std::size_t k = 1; k < along.size(); ++k) {
            const int a = along[k - 1], b = along[k];
            const float w = dist(nodes[a], nodes[b]);
            edges[a].push_back({b, w});
            edges[b].push_back({a, w});
        }
    }

    // Dijkstra from node 0 (the graph is tiny, so a linear scan for the
    // closest unvisited node is enough).
    std::vector<float> best(n, std::numeric_limits<float>::max());
    std::vector<int> previous(n, -1);
    std::vector<bool> done(n, false);
    best[0] = 0.0f;
    for (int round = 0; round < n; ++round) {
        int u = -1;
        for (int i = 0; i < n; ++i)
            if (!done[i] && (u < 0 || best[i] < best[u])) u = i;
        if (u < 0 || best[u] == std::numeric_limits<float>::max()) break;
        done[u] = true;
        for (const auto& [v, w] : edges[u]) {
            if (best[u] + w < best[v]) {
                best[v] = best[u] + w;
                previous[v] = u;
            }
        }
    }

    std::vector<Point> route = {to};
    if (best[1] == std::numeric_limits<float>::max()) {
        route.push_back(from);                     // unreachable: straight line
    } else {
        for (int v = 1; v != -1; v = previous[v]) route.push_back(nodes[v]);
        route.push_back(from);
    }
    std::reverse(route.begin(), route.end());

    // Drop repeated points (e.g. the ambulance already on a centre line).
    std::vector<Point> clean;
    for (const Point& p : route)
        if (clean.empty() || dist(clean.back(), p) > MergeDistance) clean.push_back(p);
    return clean;
}

float RouteGuide::length(const std::vector<Point>& route) {
    float total = 0.0f;
    for (std::size_t i = 1; i < route.size(); ++i) total += dist(route[i - 1], route[i]);
    return total;
}

RouteGuide::RouteGuide(const RoadNetwork& network) : network_(network) {}

void RouteGuide::update(float deltaTime, Point ambulance, Point incident, bool active) {
    active_ = active;
    phase_ = std::fmod(phase_ + FlowSpeed * deltaTime, DashLength + DashGap);
    route_ = plan(network_, ambulance, incident);
}

void RouteGuide::draw(IRenderer& target, const LineRenderer& lines,
                      Rgba colour, bool debug) const {
    if (!active_ || route_.size() < 2) return;

    // Dashes: walk the polyline by arc length; a point is "on" while
    // (s - phase) mod period < DashLength, so the pattern flows forwards.
    const float period = DashLength + DashGap;
    float s0 = 0.0f;
    for (std::size_t i = 1; i < route_.size(); ++i) {
        const Point a = route_[i - 1], b = route_[i];
        const float len = dist(a, b);
        if (len <= 0.0f) continue;
        const Point d{(b.x - a.x) / len, (b.y - a.y) / len};

        // First dash start at or after s0 on this segment.
        float start = std::ceil((s0 - phase_ - period) / period) * period + phase_;
        for (; start < s0 + len; start += period) {
            const float t0 = std::max({start, s0, StartGap}) - s0;
            const float t1 = std::min(start + DashLength, s0 + len) - s0;
            if (t1 <= t0) continue;
            const Point p0{a.x + d.x * t0, a.y + d.y * t0};
            const Point p1{a.x + d.x * t1, a.y + d.y * t1};
            lines.draw(target, {p0.x - 2, p0.y - 2}, {p1.x - 2, p1.y - 2}, Outline, 10.0f);
            lines.draw(target, p0, p1, colour, 6.0f);
        }
        s0 += len;
    }

    // Arrow beside the ambulance, pointing along the first leg of the
    // route (a triangle filled with the scan-line fill).
    const Point from = route_[0];
    Point towards = route_[1];
    for (std::size_t i = 1; i < route_.size(); ++i) {
        towards = route_[i];
        if (dist(from, towards) > ArrowDistance) break;
    }
    const float len = dist(from, towards);
    if (len > 1.0f) {
        const Point d{(towards.x - from.x) / len, (towards.y - from.y) / len};
        const Point n{-d.y, d.x};
        // Chevron of length `size` and half-width `half`, scaled up a
        // little for the dark outline drawn underneath.
        auto chevron = [&](float size, float half, float back) {
            const Point base{from.x + d.x * ArrowDistance, from.y + d.y * ArrowDistance};
            return std::vector<Point>{
                {base.x + d.x * size, base.y + d.y * size},
                {base.x + n.x * half, base.y + n.y * half},
                {base.x - d.x * back, base.y - d.y * back},
                {base.x - n.x * half, base.y - n.y * half},
            };
        };
        PolygonFill::draw(target, chevron(26, 19, 9), lines, Outline, false);
        PolygonFill::draw(target, chevron(22, 15, 5), lines, colour, false);
    }

    if (debug) {
        for (std::size_t i = 1; i + 1 < route_.size(); ++i)
            lines.draw(target, {route_[i].x - 5, route_[i].y - 5},
                       {route_[i].x + 5, route_[i].y + 5}, Colors::Debug, 1.0f);
    }
}
