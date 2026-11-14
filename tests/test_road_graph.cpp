#include "Check.h"
#include "city/City.h"
#include "city/RoadGraph.h"
#include <cmath>

namespace {

bool inside(Point p, const Rect& r) {
    return p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom;
}

void graphOfTheDistrict() {
    RoadNetwork network;
    RoadGraph graph(network);
    // 12 junctions + every road leaving the map at both ends (7 roads).
    CHECK(network.intersections().size() == 12);
    CHECK(graph.nodes().size() == 12 + 14);
    // Vertical roads: 3 junctions + 2 ends = 4 edges each; horizontal: 5 each.
    CHECK(graph.edges().size() == 4 * 4 + 3 * 5);

    // A junction in the middle of the grid has four neighbours, a map-edge
    // node exactly one.
    int maxDegree = 0, ends = 0;
    for (const GraphNode& n : graph.nodes()) {
        const int degree = static_cast<int>(graph.neighbours(n.id).size());
        maxDegree = std::max(maxDegree, degree);
        if (n.junction < 0) { CHECK(degree == 1); ++ends; }
    }
    CHECK(maxDegree == 4);
    CHECK(ends == 14);

    // Edge lengths are the distances between their nodes and every road is named.
    for (const GraphEdge& e : graph.edges()) {
        const Point a = graph.nodes()[e.from].position, b = graph.nodes()[e.to].position;
        CHECK_NEAR(e.length, std::hypot(b.x - a.x, b.y - a.y), 1e-3f);
        CHECK(!e.road.empty());
    }
}

void nearestPointOnTheRoads() {
    RoadNetwork network;
    RoadGraph graph(network);
    const auto hit = graph.nearestEdgePoint({500, 230});      // near the first street
    CHECK_NEAR(hit.point.y, 220.0f, 1e-3f);
    CHECK_NEAR(hit.point.x, 500.0f, 1e-3f);
    CHECK_NEAR(hit.distance, 10.0f, 1e-3f);
}

void incidentSpotsAreInLanes() {
    RoadNetwork network;
    RoadGraph graph(network);
    const auto spots = graph.incidentSpots();
    CHECK(!spots.empty());
    for (const Point& p : spots) {
        const auto hit = graph.nearestEdgePoint(p);
        CHECK_NEAR(hit.distance, RoadNetwork::LaneOffset, 1e-3f);   // a lane, not the centre line
        CHECK(network.junctionAt(p) < 0);                             // never inside a junction
    }
}

void cityIsBuiltFromTheBlocks() {
    RoadNetwork network;
    City city(network);
    CHECK(city.buildings().size() >= 20);
    CHECK(city.parks().size() == 3);

    // Every building sits inside a block (never on a road).
    for (const Polygon& b : city.buildings()) {
        bool ok = false;
        for (const Rect& block : network.blocks()) {
            bool all = true;
            for (const Point& p : b.vertices()) all = all && inside(p, block);
            ok = ok || all;
        }
        CHECK(ok);
    }

    // The ambulance starts in a lane right next to the hospital.
    const Point start = city.hospitalEntrance();
    RoadGraph graph(network);
    CHECK_NEAR(graph.nearestEdgePoint(start).distance, RoadNetwork::LaneOffset, 1e-3f);
    CHECK(std::abs(start.x - city.hospital().centroid().x) < 1.0f);
}

}

int main() {
    graphOfTheDistrict();
    nearestPointOnTheRoads();
    incidentSpotsAreInLanes();
    cityIsBuiltFromTheBlocks();
    return check::failures();
}
