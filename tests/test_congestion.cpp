#include "Check.h"
#include "city/RoadGraph.h"
#include "simulation/Congestion.h"
#include "simulation/Pathfinding.h"

namespace {

// 2x2 grid: junctions (300,200) (800,200) (300,520) (800,520).
RoadNetwork small() { return RoadNetwork({300, 800}, {200, 520}, 1100, 720); }

int edgeAt(const RoadGraph& g, Point p) { return g.nearestEdgePoint(p).edge; }

void emptyRoadsAreFree() {
    RoadNetwork network = small();
    RoadGraph graph(network);
    Congestion c(graph);
    c.update(1.0f, {});
    for (float f : c.factors()) CHECK_NEAR(f, 1.0f, 1e-5f);
    CHECK(c.level(0) == 0.0f);
}

void queueRaisesTheFactorSmoothly() {
    RoadNetwork network = small();
    RoadGraph graph(network);
    Congestion c(graph);
    const int top = edgeAt(graph, {550, 200});      // (300,200)-(800,200), 500 px

    // Four cars queued in the eastbound lane.
    std::vector<Congestion::Sample> queue;
    for (int k = 0; k < 4; ++k) queue.push_back({{700.0f - k * 45.0f, 165.0f}, 0.0f, false});
    const float raw = 1.0f + 4 * Congestion::QueueDelay * Congestion::ParSpeed / 500.0f;

    c.update(1.0f / 60.0f, queue);
    const float first = c.factors()[static_cast<std::size_t>(top)];
    CHECK(first > 1.0f && first < raw);              // smoothed, not a jump
    for (int i = 0; i < 600; ++i) c.update(1.0f / 60.0f, queue);
    CHECK_NEAR(c.factors()[static_cast<std::size_t>(top)], raw, 0.01f);
    CHECK(c.level(top) > 0.5f);

    // Other edges stay free.
    CHECK_NEAR(c.factors()[static_cast<std::size_t>(edgeAt(graph, {300, 360}))], 1.0f, 1e-4f);

    // The same cars pulling over for the siren count as moving.
    for (auto& s : queue) s.yielding = true;
    for (int i = 0; i < 600; ++i) c.update(1.0f / 60.0f, queue);
    CHECK(c.factors()[static_cast<std::size_t>(top)] < 1.3f);
}

void routeAvoidsTheJam() {
    RoadNetwork network = small();
    RoadGraph graph(network);
    Congestion c(graph);

    // From the west end of the top road to the east end of it: straight on
    // is 500 px; via the bottom road 320 + 500 + 320 = 1140 px.
    std::vector<Congestion::Sample> jam;
    for (int k = 0; k < 10; ++k) jam.push_back({{750.0f - k * 45.0f, 165.0f}, 0.0f, false});
    for (int i = 0; i < 900; ++i) c.update(1.0f / 60.0f, jam);

    const SearchGraph free = Pathfinding::fromRoads(graph);
    const SearchGraph jammed = Pathfinding::fromRoads(graph, &c.factors());
    const int a = graph.nodeAtJunction(0), b = graph.nodeAtJunction(1);
    const PathResult direct = Pathfinding::aStar(free, a, b);
    const PathResult around = Pathfinding::aStar(jammed, a, b);
    CHECK(direct.nodes.size() == 2);                 // straight along the top road
    CHECK(around.nodes.size() > 2);                  // the long way round
    CHECK_NEAR(around.cost, 1140.0f, 0.5f);
    CHECK(c.factors()[static_cast<std::size_t>(edgeAt(graph, {550, 200}))] <= Congestion::MaxFactor);
}

}

int main() {
    emptyRoadsAreFree();
    queueRaisesTheFactorSmoothly();
    routeAvoidsTheJam();
    return check::failures();
}
