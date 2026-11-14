#include "Check.h"
#include "city/RoadGraph.h"
#include "simulation/RouteGuide.h"
#include <cmath>

namespace {

// The original 2x2 layout, so the expected routes are easy to check by hand.
RoadNetwork small() { return RoadNetwork({300, 800}, {200, 520}, 1100, 720); }

bool contains(const std::vector<Point>& route, Point p) {
    for (const Point& q : route)
        if (std::abs(q.x - p.x) < 0.5f && std::abs(q.y - p.y) < 0.5f) return true;
    return false;
}

void sameRoadGoesStraight() {
    RoadNetwork network = small();
    RoadGraph graph(network);
    // Both on the y = 200 road, between the two junctions.
    const auto route = RouteGuide::plan(graph, {420, 175}, {560, 225});
    CHECK(route.size() == 4);                       // from, join, join, to
    CHECK_NEAR(RouteGuide::length(route), 25 + 140 + 25, 0.01f);
    CHECK(!contains(route, {300, 200}));
    CHECK(!contains(route, {800, 200}));
}

void turnsAtJunctions() {
    RoadNetwork network = small();
    RoadGraph graph(network);
    // From the hospital road to the far bottom-left: west to (300,200),
    // south to (300,520), west to the incident.
    const auto route = RouteGuide::plan(graph, {420, 175}, {120, 545});
    CHECK(contains(route, {300, 200}));
    CHECK(contains(route, {300, 520}));
    CHECK(!contains(route, {800, 200}));
    CHECK_NEAR(RouteGuide::length(route), 25 + 120 + 320 + 180 + 25, 0.01f);
}

void picksTheShorterWay() {
    RoadNetwork network = small();
    RoadGraph graph(network);
    // Start just east of (800,200); going via x = 800 is shorter than x = 300.
    const auto route = RouteGuide::plan(graph, {900, 200}, {980, 520});
    CHECK(contains(route, {800, 200}));
    CHECK(contains(route, {800, 520}));
    CHECK(!contains(route, {300, 200}));
    CHECK_NEAR(RouteGuide::length(route), 100 + 320 + 180, 0.01f);
}

void routeEndsAtBothPoints() {
    RoadNetwork network = small();
    RoadGraph graph(network);
    const auto route = RouteGuide::plan(graph, {610, 360}, {825, 70});
    CHECK(route.front().x == 610 && route.front().y == 360);
    CHECK(route.back().x == 825 && route.back().y == 70);
    // Every leg is horizontal or vertical once on the roads.
    for (std::size_t i = 2; i + 1 < route.size(); ++i) {
        const Point a = route[i - 1], b = route[i];
        CHECK(std::abs(a.x - b.x) < 0.01f || std::abs(a.y - b.y) < 0.01f);
    }
}

}

int main() {
    sameRoadGoesStraight();
    turnsAtJunctions();
    picksTheShorterWay();
    routeEndsAtBothPoints();
    return check::failures();
}
