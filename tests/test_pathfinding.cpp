#include "Check.h"
#include "city/RoadGraph.h"
#include "simulation/Pathfinding.h"
#include <cmath>

namespace {

// Hand-made graph with a known answer:
//
//   0 ---10--- 1 ---10--- 2
//   |                     |
//   4 ---------30-------- 3        5 (no edges)
//
// 0 -> 3: via 1, 2 = 10 + 10 + 10 = 30; via 4 = 10 + 30 = 40.
SearchGraph small() {
    SearchGraph g;
    g.addNode({0, 0});    // 0
    g.addNode({10, 0});   // 1
    g.addNode({20, 0});   // 2
    g.addNode({20, 10});  // 3
    g.addNode({0, 10});   // 4
    g.addNode({50, 50});  // 5, unreachable
    g.addEdge(0, 1, 10);
    g.addEdge(1, 2, 10);
    g.addEdge(2, 3, 10);
    g.addEdge(0, 4, 10);
    g.addEdge(4, 3, 30);
    return g;
}

void knownShortestPath() {
    const SearchGraph g = small();
    for (const PathResult& r : {Pathfinding::dijkstra(g, 0, 3), Pathfinding::aStar(g, 0, 3)}) {
        CHECK(r.found);
        CHECK_NEAR(r.cost, 30.0f, 1e-4f);
        CHECK(r.nodes.size() == 4);
        CHECK(r.nodes.front() == 0 && r.nodes.back() == 3);
        CHECK(r.nodes[1] == 1 && r.nodes[2] == 2);
        CHECK(r.expanded.back() == 3);          // stops when the goal is settled
    }
}

void unreachableAndInvalid() {
    const SearchGraph g = small();
    CHECK(!Pathfinding::dijkstra(g, 0, 5).found);
    CHECK(!Pathfinding::aStar(g, 0, 5).found);
    CHECK(Pathfinding::aStar(g, 0, 5).nodes.empty());
    CHECK(!Pathfinding::dijkstra(g, 0, 99).found);
    // Start == goal: found, zero cost, one node.
    const PathResult self = Pathfinding::aStar(g, 2, 2);
    CHECK(self.found && self.cost == 0.0f && self.nodes.size() == 1);
}

void aStarMatchesDijkstraOnTheDistrict() {
    RoadNetwork network;
    RoadGraph roads(network);
    const SearchGraph g = Pathfinding::fromRoads(roads);

    // Every pair of nodes: same cost, and A* never settles more nodes.
    int fewer = 0, pairs = 0;
    for (int a = 0; a < g.size(); ++a) {
        for (int b = 0; b < g.size(); ++b) {
            const PathResult d = Pathfinding::dijkstra(g, a, b);
            const PathResult s = Pathfinding::aStar(g, a, b);
            CHECK(d.found && s.found);
            CHECK_NEAR(d.cost, s.cost, 0.01f);
            CHECK(s.expanded.size() <= d.expanded.size());
            if (s.expanded.size() < d.expanded.size()) ++fewer;
            ++pairs;
        }
    }
    // The heuristic must actually pay off on most queries.
    CHECK(fewer * 2 > pairs);
}

void joiningTheRoads() {
    RoadNetwork network({300, 800}, {200, 520}, 1100, 720);
    RoadGraph roads(network);
    SearchGraph g = Pathfinding::fromRoads(roads);
    const int base = g.size();

    // Two points between the same two junctions are linked directly.
    const int a = Pathfinding::joinRoad(g, roads, {420, 175});
    const int b = Pathfinding::joinRoad(g, roads, {560, 225});
    CHECK(a == base && b == base + 1);
    CHECK_NEAR(g.positions[a].y, 200.0f, 1e-3f);
    const PathResult r = Pathfinding::aStar(g, a, b);
    CHECK_NEAR(r.cost, 140.0f, 0.01f);
    CHECK(r.nodes.size() == 2);

    // Congestion factors scale the edge weights.
    std::vector<float> slow(roads.edges().size(), 3.0f);
    SearchGraph jammed = Pathfinding::fromRoads(roads, &slow);
    const int ja = Pathfinding::joinRoad(jammed, roads, {420, 175}, &slow);
    const int jb = Pathfinding::joinRoad(jammed, roads, {560, 225}, &slow);
    CHECK_NEAR(Pathfinding::aStar(jammed, ja, jb).cost, 420.0f, 0.05f);
}

}

int main() {
    knownShortestPath();
    unreachableAndInvalid();
    aStarMatchesDijkstraOnTheDistrict();
    joiningTheRoads();
    return check::failures();
}
