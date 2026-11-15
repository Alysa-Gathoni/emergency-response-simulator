#include "RouteGuide.h"
#include "../core/Color.h"
#include "../rasterization/PolygonFill.h"
#include "../shapes/CircleEllipse.h"
#include "../ui/Hud.h"
#include "../ui/PixelFont.h"
#include <string>
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

// Route polyline: from, the search nodes, to; points closer than
// MergeDistance merged (e.g. the ambulance already on a centre line).
std::vector<Point> toPolyline(const SearchGraph& graph, const PathResult& path,
                              Point from, Point to) {
    std::vector<Point> route = {from};
    if (path.found)
        for (int node : path.nodes) route.push_back(graph.positions[static_cast<std::size_t>(node)]);
    route.push_back(to);

    std::vector<Point> clean;
    for (const Point& p : route)
        if (clean.empty() || dist(clean.back(), p) > MergeDistance) clean.push_back(p);
    return clean;
}

}

std::vector<Point> RouteGuide::plan(const RoadGraph& graph, Point from, Point to) {
    SearchGraph search = Pathfinding::fromRoads(graph);
    const int start = Pathfinding::joinRoad(search, graph, from);
    const int goal = Pathfinding::joinRoad(search, graph, to);
    return toPolyline(search, Pathfinding::aStar(search, start, goal), from, to);
}

std::vector<Point> RouteGuide::planWithSearch(Point from, Point to) {
    search_ = Pathfinding::fromRoads(graph_, factors_);
    const int start = Pathfinding::joinRoad(search_, graph_, from, factors_);
    const int goal = Pathfinding::joinRoad(search_, graph_, to, factors_);
    aStar_ = Pathfinding::aStar(search_, start, goal);
    if (compare_) dijkstra_ = Pathfinding::dijkstra(search_, start, goal);
    return toPolyline(search_, aStar_, from, to);
}

float RouteGuide::length(const std::vector<Point>& route) {
    float total = 0.0f;
    for (std::size_t i = 1; i < route.size(); ++i) total += dist(route[i - 1], route[i]);
    return total;
}

RouteGuide::RouteGuide(const RoadGraph& graph) : graph_(graph) {}

float RouteGuide::distanceTo(const std::vector<Point>& route, Point p, int* segment, Point* closest) {
    float best = std::numeric_limits<float>::max();
    if (route.size() < 2) return route.empty() ? best : dist(route.front(), p);
    for (std::size_t i = 1; i < route.size(); ++i) {
        const Point a = route[i - 1], b = route[i];
        const Point ab{b.x - a.x, b.y - a.y};
        const float len2 = ab.x * ab.x + ab.y * ab.y;
        float t = len2 > 0.0f ? ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / len2 : 0.0f;
        t = std::clamp(t, 0.0f, 1.0f);
        const Point q{a.x + ab.x * t, a.y + ab.y * t};
        const float d = dist(p, q);
        if (d < best) {
            best = d;
            if (segment) *segment = static_cast<int>(i) - 1;
            if (closest) *closest = q;
        }
    }
    return best;
}

void RouteGuide::update(float deltaTime, Point ambulance, Point target, bool active) {
    active_ = active;
    phase_ = std::fmod(phase_ + FlowSpeed * deltaTime, DashLength + DashGap);
    notice_ = std::max(0.0f, notice_ - deltaTime);
    sinceRefresh_ += deltaTime;
    legAge_ += deltaTime;

    const bool newTarget = !havePlan_ || dist(target, planTarget_) > 1.0f;
    bool wrongTurn = false;
    std::vector<Point> trimmed;
    if (!newTarget && plan_.size() >= 2) {
        // Follow the committed plan (without the old start connector): cut
        // off the part already driven at the ambulance's projection.
        int segment = 0;
        Point closest;
        const float off = distanceTo(plan_, ambulance, &segment, &closest);

        // Wrong way: moving against the route's direction where the
        // ambulance is, for WrongWayTime in a row.
        const Point a = plan_[static_cast<std::size_t>(segment)];
        const Point b = plan_[static_cast<std::size_t>(segment) + 1];
        const float segLen = dist(a, b);
        const Point move{ambulance.x - lastPosition_.x, ambulance.y - lastPosition_.y};
        const float moved = std::hypot(move.x, move.y);
        if (segLen > 0.0f && moved > 0.3f) {
            const float along = (move.x * (b.x - a.x) + move.y * (b.y - a.y)) / (segLen * moved);
            wrongWayTime_ = along < -0.5f ? wrongWayTime_ + deltaTime : 0.0f;
        }
        std::vector<Point> rest = {closest};
        rest.insert(rest.end(), plan_.begin() + segment + 1, plan_.end());
        plan_.clear();
        for (const Point& p : rest)
            if (plan_.empty() || dist(plan_.back(), p) > MergeDistance) plan_.push_back(p);

        trimmed = {ambulance};
        for (const Point& p : plan_)
            if (dist(trimmed.back(), p) > MergeDistance) trimmed.push_back(p);

        wrongTurn = active && (off > OffRoute || wrongWayTime_ > WrongWayTime);
    }
    lastPosition_ = ambulance;

    if (newTarget || wrongTurn || sinceRefresh_ >= Refresh || trimmed.size() < 2) {
        route_ = planWithSearch(ambulance, target);
        plan_.assign(route_.begin() + (route_.size() > 1 ? 1 : 0), route_.end());
        havePlan_ = true;
        planTarget_ = target;
        sinceRefresh_ = 0.0f;
        if (newTarget) legAge_ = 0.0f;
        if (wrongTurn) {
            wrongWayTime_ = 0.0f;
            // Turning round at the start of a leg is not a mistake.
            if (legAge_ >= Grace) {
                ++wrongTurns_;
                notice_ = RecalcNotice;
            }
        }
    } else {
        route_ = trimmed;
    }
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

void RouteGuide::drawComparison(IRenderer& target, const LineRenderer& lines) const {
    if (!compare_ || !active_) return;

    // Dijkstra: blue rings round every settled node.
    const Rgba dijkstraColour(90, 160, 255);
    for (int node : dijkstra_.expanded) {
        const Point p = search_.positions[static_cast<std::size_t>(node)];
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::midpointCircle(p, 22),
                                    dijkstraColour, 3.0f);
    }
    // A*: orange discs inside the rings, numbered in the order they were settled.
    const Rgba aStarColour(255, 170, 60);
    for (std::size_t k = 0; k < aStar_.expanded.size(); ++k) {
        const Point p = search_.positions[static_cast<std::size_t>(aStar_.expanded[k])];
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(p, 13), aStarColour, 1.0f);
        const std::string n = std::to_string(k + 1);
        PixelFont::drawText(target, n, {p.x - PixelFont::textWidth(n, 1) * 0.5f, p.y - 3.0f}, 1,
                            Rgba(20, 24, 30));
    }
}

void RouteGuide::drawComparisonPanel(IRenderer& target, const LineRenderer& lines,
                                     Point topLeft) const {
    if (!compare_ || !active_) return;
    const float w = 300.0f, h = 94.0f;
    const std::vector<Point> panel = {topLeft, {topLeft.x + w, topLeft.y},
                                      {topLeft.x + w, topLeft.y + h}, {topLeft.x, topLeft.y + h}};
    PolygonFill::draw(target, panel, lines, Rgba(16, 20, 28, 220), false);

    auto line = [&](float y, const std::string& text, Rgba c) {
        PixelFont::drawText(target, text, {topLeft.x + 12.0f, topLeft.y + y}, 2, c);
    };
    const bool same = std::abs(dijkstra_.cost - aStar_.cost) < 0.5f;
    line(10, "PATHFINDING (V)", Rgba(230, 230, 230));
    line(32, "DIJKSTRA " + std::to_string(dijkstra_.expanded.size()) + " NODES", Rgba(90, 160, 255));
    line(52, "A-STAR   " + std::to_string(aStar_.expanded.size()) + " NODES", Rgba(255, 170, 60));
    line(72, same ? "SAME COST " + std::to_string(static_cast<int>(std::lround(aStar_.cost * MetresPerPixel)))
                  : std::string("COSTS DIFFER"),
         same ? Rgba(120, 220, 150) : Rgba(235, 80, 70));
}
