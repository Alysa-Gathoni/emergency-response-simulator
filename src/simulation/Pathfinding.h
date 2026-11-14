#pragma once
#include <limits>
#include <utility>
#include <vector>
#include "../core/Point.h"

class RoadGraph;

// Shortest paths on a weighted graph with non-negative edge weights.
//
// Dijkstra settles nodes in order of their distance g(n) from the start;
// the first time the goal is settled its distance is final.
//
// A* settles nodes in order of f(n) = g(n) + h(n), where h is the straight
// line distance from n to the goal. On roads every edge weight is at least
// its length (weights only go up with congestion), so h never overestimates
// (admissible) and h(a) <= w(a, b) + h(b) (consistent): A* finds the same
// cost as Dijkstra but settles fewer nodes, because nodes leading away
// from the goal get a large f and are never expanded.
//
// Both use a binary heap (std::priority_queue) with lazy deletion:
// O((V + E) log V).
struct SearchGraph {
    std::vector<Point> positions;
    std::vector<std::vector<std::pair<int, float>>> adjacency;   // (node, weight)

    int addNode(Point p);
    void addEdge(int a, int b, float weight);    // both directions
    int size() const { return static_cast<int>(positions.size()); }
};

struct PathResult {
    bool found{false};
    float cost{std::numeric_limits<float>::max()};
    std::vector<int> nodes;      // start .. goal
    std::vector<int> expanded;   // nodes in the order they were settled
};

namespace Pathfinding {

PathResult dijkstra(const SearchGraph& graph, int start, int goal);
PathResult aStar(const SearchGraph& graph, int start, int goal);

// The road graph as a search graph (node ids kept). `edgeFactor`, if given,
// multiplies each road edge's length (congestion); missing = 1.
SearchGraph fromRoads(const RoadGraph& roads, const std::vector<float>* edgeFactor = nullptr);

// Adds a node where `p` joins its nearest road (on the centre line),
// connected to both ends of that road section. Returns the new node id.
int joinRoad(SearchGraph& graph, const RoadGraph& roads, Point p,
             const std::vector<float>* edgeFactor = nullptr);

}
