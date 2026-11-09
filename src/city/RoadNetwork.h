#pragma once
#include <utility>
#include <vector>
#include "Direction.h"
#include "../core/Point.h"
#include "../curves/Spline.h"

struct Rect {
    float left;
    float top;
    float right;
    float bottom;
};

// A straight two-lane road crossing the whole map.
struct Road {
    bool vertical;   // vertical roads run north-south
    float centre;    // x of a vertical road, y of a horizontal road
};

struct Intersection {
    Point centre;
};

// Vehicles travelling in direction `approach` must stop before the
// segment a-b when their signal group is not green.
struct StopLine {
    int intersection;
    Heading approach;
    Point a;   // on the road centre line
    Point b;   // on the kerb
};

// Zebra crossing on one arm of an intersection.
struct Crosswalk {
    int intersection;
    Heading arm;              // North = the arm above the intersection centre
    Point start;              // walking line, kerb to kerb
    Point end;
    SignalGroup trafficGroup; // vehicles that drive over this crossing
};

// Road layout of the district: a 2x2 grid of signalised intersections.
// Every other module (city drawing, traffic lights, traffic routes,
// pedestrians) derives its geometry from here, so the map is defined once.
class RoadNetwork {
public:
    static constexpr float WorldWidth = 1100.0f;
    static constexpr float WorldHeight = 720.0f;
    static constexpr float HalfWidth = 70.0f;        // road centre to kerb (140 px roads)
    static constexpr float LaneOffset = 35.0f;       // road centre to lane centre
    static constexpr float CrosswalkInner = 72.0f;   // distances from the
    static constexpr float CrosswalkOuter = 88.0f;   // intersection centre
    static constexpr float StopLineDistance = 94.0f;

    RoadNetwork();

    const std::vector<Road>& roads() const;
    const std::vector<Intersection>& intersections() const;
    const std::vector<StopLine>& stopLines() const;
    const std::vector<Crosswalk>& crosswalks() const;

    // Land between the roads (pavement, buildings and parks go here).
    std::vector<Rect> blocks() const;

    // Parts of a road between intersection boxes, as intervals along the
    // road's own axis.
    std::vector<std::pair<float, float>> freeIntervals(const Road& road) const;

    // Centre of the lane used by traffic travelling in direction `travel`
    // past `point` (left-hand traffic).
    static Point laneCentre(Point point, Point travel);

    // Builds a closed route through axis-aligned waypoints (intersection
    // centres, or road ends just off the map). Consecutive waypoints must
    // share a road. Turns become cubic Bezier corners and road ends become
    // U-turns, so the result is one continuous C1 path in the correct lane.
    static SplinePath buildRoute(const std::vector<Point>& waypoints);

private:
    std::vector<Road> roads_;
    std::vector<Intersection> intersections_;
    std::vector<StopLine> stopLines_;
    std::vector<Crosswalk> crosswalks_;
};
