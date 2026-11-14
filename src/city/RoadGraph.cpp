#include "RoadGraph.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
float dist(Point a, Point b) { return std::hypot(b.x - a.x, b.y - a.y); }
}

RoadGraph::RoadGraph(const RoadNetwork& network) : network_(network) {
    // Nodes: intersections first (node id == junction index), then road ends.
    const auto& junctions = network.intersections();
    for (int i = 0; i < static_cast<int>(junctions.size()); ++i)
        nodes_.push_back({i, junctions[static_cast<std::size_t>(i)].centre, i});

    auto addNode = [&](Point p) {
        for (const GraphNode& n : nodes_)
            if (dist(n.position, p) < 0.5f) return n.id;
        const int id = static_cast<int>(nodes_.size());
        nodes_.push_back({id, p, -1});
        return id;
    };

    for (const Road& road : network.roads()) {
        const float extent = road.vertical ? network.height() : network.width();
        const Point start = road.vertical ? Point{road.centre, 0.0f} : Point{0.0f, road.centre};
        const Point end = road.vertical ? Point{road.centre, extent} : Point{extent, road.centre};

        // Every node on this road, sorted along it.
        std::vector<int> along = {addNode(start), addNode(end)};
        for (const GraphNode& n : nodes_) {
            if (n.junction < 0) continue;
            const bool on = road.vertical ? std::abs(n.position.x - road.centre) < 0.5f
                                          : std::abs(n.position.y - road.centre) < 0.5f;
            if (on) along.push_back(n.id);
        }
        std::sort(along.begin(), along.end(), [&](int a, int b) {
            return road.vertical ? nodes_[a].position.y < nodes_[b].position.y
                                 : nodes_[a].position.x < nodes_[b].position.x;
        });
        along.erase(std::unique(along.begin(), along.end()), along.end());

        const float limit = road.name.find("HIGHWAY") != std::string::npos ? 80.0f : 50.0f;
        for (std::size_t k = 1; k < along.size(); ++k) {
            const int a = along[k - 1], b = along[k];
            edges_.push_back({a, b, dist(nodes_[a].position, nodes_[b].position), 2, limit, road.name});
        }
    }

    adjacency_.resize(nodes_.size());
    for (int e = 0; e < static_cast<int>(edges_.size()); ++e) {
        adjacency_[edges_[e].from].push_back({e, edges_[e].to});
        adjacency_[edges_[e].to].push_back({e, edges_[e].from});
    }
}

const std::vector<GraphNode>& RoadGraph::nodes() const { return nodes_; }
const std::vector<GraphEdge>& RoadGraph::edges() const { return edges_; }

const std::vector<std::pair<int, int>>& RoadGraph::neighbours(int node) const {
    return adjacency_[static_cast<std::size_t>(node)];
}

int RoadGraph::nodeAtJunction(int junction) const {
    for (const GraphNode& n : nodes_)
        if (n.junction == junction) return n.id;
    return -1;
}

RoadGraph::EdgePoint RoadGraph::nearestEdgePoint(Point p) const {
    EdgePoint best;
    best.distance = std::numeric_limits<float>::max();
    for (int e = 0; e < static_cast<int>(edges_.size()); ++e) {
        const Point a = nodes_[edges_[e].from].position;
        const Point b = nodes_[edges_[e].to].position;
        const Point ab{b.x - a.x, b.y - a.y};
        const float len2 = ab.x * ab.x + ab.y * ab.y;
        float t = len2 > 0.0f ? ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / len2 : 0.0f;
        t = std::clamp(t, 0.0f, 1.0f);
        const Point q{a.x + ab.x * t, a.y + ab.y * t};
        const float d = dist(p, q);
        if (d < best.distance) best = {e, t, q, d};
    }
    return best;
}

std::vector<Point> RoadGraph::incidentSpots() const {
    std::vector<Point> spots;
    for (const GraphEdge& e : edges_) {
        const GraphNode& a = nodes_[e.from];
        const GraphNode& b = nodes_[e.to];
        if (a.junction < 0 || b.junction < 0) continue;   // edges leaving the map
        const Point mid{(a.position.x + b.position.x) * 0.5f, (a.position.y + b.position.y) * 0.5f};
        const Point d{(b.position.x - a.position.x) / e.length, (b.position.y - a.position.y) / e.length};
        spots.push_back(RoadNetwork::laneCentre(mid, d));
        spots.push_back(RoadNetwork::laneCentre(mid, {-d.x, -d.y}));
    }
    return spots;
}
