#pragma once
#include "render/IRenderer.h"
#include <vector>
#include "../city/RoadGraph.h"
#include "Pathfinding.h"
#include "../lines/LineRenderer.h"

// Shortest route along the roads from the ambulance to the incident.
//
// Search graph = the RoadGraph (junctions and map-edge points, edges
// weighted by length times the live congestion factor) plus two extra nodes where the ambulance and the
// target join their nearest road section. A* with the straight-line
// heuristic finds the route.
//
// The planned route is kept while the ambulance follows it (the part
// already driven is trimmed off by projecting the ambulance onto the
// polyline). It is planned again when
//   - the target changes (new call, patient on board),
//   - the ambulance is more than OffRoute px from the route (a wrong turn),
//   - the ambulance has been moving against the route's direction for
//     WrongWayTime seconds (driving the wrong way along it),
//   - every Refresh seconds, so new congestion is taken into account.
// The middle two are counted as wrong turns and flagged for the banner,
// except during the first Grace seconds of a leg (turning round to start).
//
// V shows the comparison overlay: the same query is also answered by
// Dijkstra, and the nodes each algorithm settled are marked on the map
// (Dijkstra rings, A* discs numbered in settling order) with the counts
// in a panel.
//
// The route is drawn as dashes flowing towards the target, plus an arrow
// next to the ambulance pointing along the first leg.
class RouteGuide {
public:
    // Polyline from `from` to `to` (both included) following the roads.
    static std::vector<Point> plan(const RoadGraph& graph, Point from, Point to);
    static float length(const std::vector<Point>& route);

    static constexpr float OffRoute = 95.0f;    // px from the route (lane + margin)
    static constexpr float WrongWayTime = 2.5f; // s moving against the route
    static constexpr float Grace = 5.0f;        // s at the start of a leg
    static constexpr float Refresh = 4.0f;      // s between congestion re-plans
    static constexpr float RecalcNotice = 2.0f; // s the banner shows it

    explicit RouteGuide(const RoadGraph& graph);

    // Wrong turns since the last reset, and whether one just happened.
    int wrongTurns() const { return wrongTurns_; }
    bool recalculating() const { return notice_ > 0.0f; }
    void resetCount() { wrongTurns_ = 0; notice_ = 0.0f; }

    // Closest point of a polyline to p: distance, and the index of the
    // segment it lies on (for the trimming above).
    static float distanceTo(const std::vector<Point>& route, Point p, int* segment = nullptr,
                            Point* closest = nullptr);

    void update(float deltaTime, Point ambulance, Point target, bool active);
    void draw(IRenderer& target, const LineRenderer& lines,
              Rgba colour, bool debug) const;

    // Per-edge weight factors (congestion), >= 1; null = plain lengths.
    void setEdgeFactors(const std::vector<float>* factors) { factors_ = factors; }

    // Comparison overlay (V): world-space markers and the screen panel.
    void setCompare(bool on) { compare_ = on; }
    bool comparing() const { return compare_; }
    void drawComparison(IRenderer& target, const LineRenderer& lines) const;
    void drawComparisonPanel(IRenderer& target, const LineRenderer& lines, Point topLeft) const;

    const PathResult& lastDijkstra() const { return dijkstra_; }
    const PathResult& lastAStar() const { return aStar_; }

    const std::vector<Point>& route() const { return route_; }
    float remaining() const { return length(route_); }

private:
    const RoadGraph& graph_;
    std::vector<Point> route_;
    float phase_{0.0f};
    bool active_{false};

    bool compare_{false};
    const std::vector<float>* factors_{nullptr};
    SearchGraph search_;
    PathResult dijkstra_;
    PathResult aStar_;

    std::vector<Point> planWithSearch(Point from, Point to);

    std::vector<Point> plan_;     // committed route from the first road point on
    bool havePlan_{false};
    Point planTarget_{};
    Point lastPosition_{};
    float wrongWayTime_{0.0f};
    float legAge_{0.0f};
    float sinceRefresh_{0.0f};
    int wrongTurns_{0};
    float notice_{0.0f};
};
