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
// heuristic finds the route; it is re-planned every frame, so it follows
// the ambulance if the driver takes a different turn.
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

    explicit RouteGuide(const RoadGraph& graph);

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
};
