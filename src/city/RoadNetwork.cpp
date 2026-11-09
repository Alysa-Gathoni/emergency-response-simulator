#include "RoadNetwork.h"
#include <algorithm>
#include <cmath>

namespace {

// 4/3 * tan(pi/8): control-point distance (as a fraction of the radius)
// that makes a cubic Bezier approximate a quarter circle.
constexpr float QuarterCircleK = 0.5523f;

Point add(Point a, Point b) { return {a.x + b.x, a.y + b.y}; }
Point sub(Point a, Point b) { return {a.x - b.x, a.y - b.y}; }
Point mul(Point a, float s) { return {a.x * s, a.y * s}; }
float dot(Point a, Point b) { return a.x * b.x + a.y * b.y; }

Point unitAxis(Point from, Point to) {
    const Point d = sub(to, from);
    // Waypoints are axis aligned, so the dominant axis is the direction.
    if (std::abs(d.x) >= std::abs(d.y))
        return {d.x >= 0.0f ? 1.0f : -1.0f, 0.0f};
    return {0.0f, d.y >= 0.0f ? 1.0f : -1.0f};
}

BezierSegment straight(Point a, Point b) {
    return {a, add(a, mul(sub(b, a), 1.0f / 3.0f)),
               add(a, mul(sub(b, a), 2.0f / 3.0f)), b};
}

}

RoadNetwork::RoadNetwork() {
    roads_ = {
        {true, 300.0f}, {true, 800.0f},     // vertical avenues
        {false, 200.0f}, {false, 520.0f}    // horizontal streets
    };

    // Intersections in reading order: top-left, top-right,
    // bottom-left, bottom-right.
    for (const Road& h : roads_) {
        if (h.vertical) continue;
        for (const Road& v : roads_) {
            if (!v.vertical) continue;
            intersections_.push_back({{v.centre, h.centre}});
        }
    }

    const Heading all[] = {Heading::North, Heading::East,
                           Heading::South, Heading::West};

    for (int i = 0; i < static_cast<int>(intersections_.size()); ++i) {
        const Point c = intersections_[i].centre;

        for (Heading travel : all) {
            // Traffic travelling `travel` arrives from the opposite side,
            // in the lane to the left of its direction of travel.
            const Point d = headingVector(travel);
            const Point a = sub(c, mul(d, StopLineDistance));
            const Point b = add(a, mul(leftOf(d), HalfWidth));
            stopLines_.push_back({i, travel, a, b});
        }

        for (Heading arm : all) {
            const Point d = headingVector(arm);
            const Point mid = add(c, mul(d, (CrosswalkInner + CrosswalkOuter) * 0.5f));
            const Point across = leftOf(d);   // perpendicular to the arm
            const float reach = HalfWidth + 10.0f;
            crosswalks_.push_back({
                i, arm,
                sub(mid, mul(across, reach)),
                add(mid, mul(across, reach)),
                groupOf(arm)
            });
        }
    }
}

const std::vector<Road>& RoadNetwork::roads() const { return roads_; }
const std::vector<Intersection>& RoadNetwork::intersections() const { return intersections_; }
const std::vector<StopLine>& RoadNetwork::stopLines() const { return stopLines_; }
const std::vector<Crosswalk>& RoadNetwork::crosswalks() const { return crosswalks_; }

std::vector<Rect> RoadNetwork::blocks() const {
    std::vector<float> xs = {0.0f};
    std::vector<float> ys = {0.0f};
    for (const Road& r : roads_) {
        auto& cuts = r.vertical ? xs : ys;
        cuts.push_back(r.centre - HalfWidth);
        cuts.push_back(r.centre + HalfWidth);
    }
    xs.push_back(WorldWidth);
    ys.push_back(WorldHeight);
    std::sort(xs.begin(), xs.end());
    std::sort(ys.begin(), ys.end());

    std::vector<Rect> result;
    for (std::size_t j = 0; j + 1 < ys.size(); j += 2)
        for (std::size_t i = 0; i + 1 < xs.size(); i += 2)
            result.push_back({xs[i], ys[j], xs[i + 1], ys[j + 1]});
    return result;
}

std::vector<std::pair<float, float>>
RoadNetwork::freeIntervals(const Road& road) const {
    std::vector<float> crossings;
    for (const Road& other : roads_)
        if (other.vertical != road.vertical)
            crossings.push_back(other.centre);
    std::sort(crossings.begin(), crossings.end());

    const float length = road.vertical ? WorldHeight : WorldWidth;
    std::vector<std::pair<float, float>> result;
    float start = 0.0f;
    for (float c : crossings) {
        result.push_back({start, c - HalfWidth});
        start = c + HalfWidth;
    }
    result.push_back({start, length});
    return result;
}

Point RoadNetwork::laneCentre(Point point, Point travel) {
    return add(point, mul(leftOf(travel), LaneOffset));
}

SplinePath RoadNetwork::buildRoute(const std::vector<Point>& waypoints) {
    SplinePath path;
    const std::size_t n = waypoints.size();
    if (n < 2) return path;

    std::vector<BezierSegment> segments;
    Point previousExit{};
    Point firstEntry{};

    for (std::size_t i = 0; i < n; ++i) {
        const Point prev = waypoints[(i + n - 1) % n];
        const Point cur = waypoints[i];
        const Point next = waypoints[(i + 1) % n];

        const Point dIn = unitAxis(prev, cur);
        const Point dOut = unitAxis(cur, next);
        const float turn = dot(dIn, dOut);

        BezierSegment corner;
        if (turn < -0.5f) {
            // U-turn at a road end (off the map): a half circle of radius
            // LaneOffset joining the two lanes; cubic control distance 4/3 r.
            const Point entry = laneCentre(cur, dIn);
            const Point exit = laneCentre(cur, dOut);
            const Point bulge = mul(dIn, LaneOffset * 4.0f / 3.0f);
            corner = {entry, add(entry, bulge), add(exit, bulge), exit};
        } else {
            // Enter at the edge of the intersection box in the incoming lane,
            // leave at the opposite edge in the outgoing lane.
            const Point entry = laneCentre(sub(cur, mul(dIn, HalfWidth)), dIn);
            const Point exit = laneCentre(add(cur, mul(dOut, HalfWidth)), dOut);

            if (turn > 0.5f) {
                corner = straight(entry, exit);
            } else {
                // The incoming and outgoing lane lines meet at cornerPoint.
                // Pulling both control points towards it by 0.5523 of the
                // distance gives a quarter-circle turn. With left-hand
                // traffic, left turns come out tight (r = 25) and right
                // turns wide (r = 75) purely from the lane offsets.
                const Point cornerPoint =
                    add(entry, mul(dIn, dot(sub(exit, entry), dIn)));
                corner = {
                    entry,
                    add(entry, mul(sub(cornerPoint, entry), QuarterCircleK)),
                    add(exit, mul(sub(cornerPoint, exit), QuarterCircleK)),
                    exit
                };
            }
        }

        if (i == 0) firstEntry = corner.p0;
        else segments.push_back(straight(previousExit, corner.p0));

        segments.push_back(corner);
        previousExit = corner.p3;
    }
    segments.push_back(straight(previousExit, firstEntry));

    path.setSegments(std::move(segments));
    return path;
}
