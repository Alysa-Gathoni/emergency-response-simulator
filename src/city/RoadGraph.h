#pragma once
#include <string>
#include <utility>
#include <vector>
#include "RoadNetwork.h"

// The road network as a weighted graph, built from RoadNetwork:
//
//   nodes  every intersection, plus every point where a road leaves the map
//   edges  consecutive nodes along the same road, both directions usable,
//          weight = length in pixels
//
// Pathfinding, incident placement and the minimap all use this graph, so a
// new map only has to change the list of roads.
struct GraphNode {
    int id;
    Point position;
    int junction;          // index into RoadNetwork::intersections(), -1 = map edge
};

struct GraphEdge {
    int from;
    int to;
    float length;          // pixels
    int lanes;             // one each way
    float speedLimit;      // km/h, for display and route weighting
    std::string road;      // name of the road it belongs to
};

class RoadGraph {
public:
    explicit RoadGraph(const RoadNetwork& network);

    const std::vector<GraphNode>& nodes() const;
    const std::vector<GraphEdge>& edges() const;

    // (edge index, node at the other end) for every edge touching `node`.
    const std::vector<std::pair<int, int>>& neighbours(int node) const;

    int nodeAtJunction(int junction) const;

    // Closest point on any edge (on the road centre line).
    struct EdgePoint {
        int edge{-1};
        float t{0.0f};          // 0 at `from`, 1 at `to`
        Point point{};
        float distance{0.0f};   // from the query point
    };
    EdgePoint nearestEdgePoint(Point p) const;

    // Places where an incident can happen: the middle of every edge that
    // lies fully inside the map, in each of its two lanes.
    std::vector<Point> incidentSpots() const;

private:
    const RoadNetwork& network_;
    std::vector<GraphNode> nodes_;
    std::vector<GraphEdge> edges_;
    std::vector<std::vector<std::pair<int, int>>> adjacency_;
};
