#include "Pathfinding.h"
#include "../city/RoadGraph.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <queue>

int SearchGraph::addNode(Point p) {
    positions.push_back(p);
    adjacency.emplace_back();
    return size() - 1;
}

void SearchGraph::addEdge(int a, int b, float weight) {
    adjacency[static_cast<std::size_t>(a)].push_back({b, weight});
    adjacency[static_cast<std::size_t>(b)].push_back({a, weight});
}

namespace Pathfinding {

namespace {

constexpr float Infinity = std::numeric_limits<float>::max();

// Best-first search shared by both algorithms: the heap is ordered by
// g + h; Dijkstra is the special case h = 0.
PathResult search(const SearchGraph& graph, int start, int goal,
                  const std::function<float(int)>& heuristic) {
    PathResult result;
    const int n = graph.size();
    if (start < 0 || goal < 0 || start >= n || goal >= n) return result;

    std::vector<float> g(static_cast<std::size_t>(n), Infinity);
    std::vector<int> previous(static_cast<std::size_t>(n), -1);
    std::vector<bool> settled(static_cast<std::size_t>(n), false);

    using Entry = std::pair<float, int>;   // (f, node), smallest f on top
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;
    g[static_cast<std::size_t>(start)] = 0.0f;
    open.push({heuristic(start), start});

    while (!open.empty()) {
        const int u = open.top().second;
        open.pop();
        if (settled[static_cast<std::size_t>(u)]) continue;   // stale heap entry
        settled[static_cast<std::size_t>(u)] = true;
        result.expanded.push_back(u);
        if (u == goal) break;

        for (const auto& [v, w] : graph.adjacency[static_cast<std::size_t>(u)]) {
            const float candidate = g[static_cast<std::size_t>(u)] + w;
            if (candidate < g[static_cast<std::size_t>(v)]) {
                g[static_cast<std::size_t>(v)] = candidate;
                previous[static_cast<std::size_t>(v)] = u;
                open.push({candidate + heuristic(v), v});
            }
        }
    }

    if (g[static_cast<std::size_t>(goal)] == Infinity) return result;
    result.found = true;
    result.cost = g[static_cast<std::size_t>(goal)];
    for (int v = goal; v != -1; v = previous[static_cast<std::size_t>(v)])
        result.nodes.push_back(v);
    std::reverse(result.nodes.begin(), result.nodes.end());
    return result;
}

float factor(const std::vector<float>* edgeFactor, int edge) {
    if (!edgeFactor || edge < 0 || edge >= static_cast<int>(edgeFactor->size())) return 1.0f;
    return (*edgeFactor)[static_cast<std::size_t>(edge)];
}

}

PathResult dijkstra(const SearchGraph& graph, int start, int goal) {
    return search(graph, start, goal, [](int) { return 0.0f; });
}

PathResult aStar(const SearchGraph& graph, int start, int goal) {
    if (goal < 0 || goal >= graph.size()) return {};
    const Point target = graph.positions[static_cast<std::size_t>(goal)];
    return search(graph, start, goal, [&](int node) {
        const Point p = graph.positions[static_cast<std::size_t>(node)];
        return std::hypot(target.x - p.x, target.y - p.y);
    });
}

SearchGraph fromRoads(const RoadGraph& roads, const std::vector<float>* edgeFactor) {
    SearchGraph graph;
    for (const GraphNode& node : roads.nodes()) graph.addNode(node.position);
    const auto& edges = roads.edges();
    for (int e = 0; e < static_cast<int>(edges.size()); ++e)
        graph.addEdge(edges[e].from, edges[e].to, edges[e].length * factor(edgeFactor, e));
    return graph;
}

int joinRoad(SearchGraph& graph, const RoadGraph& roads, Point p,
             const std::vector<float>* edgeFactor) {
    const RoadGraph::EdgePoint hit = roads.nearestEdgePoint(p);
    const int id = graph.addNode(hit.point);
    if (hit.edge < 0) return id;

    const GraphEdge& edge = roads.edges()[static_cast<std::size_t>(hit.edge)];
    const float k = factor(edgeFactor, hit.edge);
    graph.addEdge(id, edge.from, edge.length * hit.t * k);
    graph.addEdge(id, edge.to, edge.length * (1.0f - hit.t) * k);

    // An earlier join on the same road section is reachable directly
    // (start and goal between the same two junctions).
    for (int other = static_cast<int>(roads.nodes().size()); other < id; ++other) {
        const RoadGraph::EdgePoint o = roads.nearestEdgePoint(graph.positions[static_cast<std::size_t>(other)]);
        if (o.edge == hit.edge)
            graph.addEdge(id, other, edge.length * std::abs(o.t - hit.t) * k);
    }
    return id;
}

}
