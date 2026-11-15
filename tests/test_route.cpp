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

void followsThenRecalculates() {
    RoadNetwork network = small();
    RoadGraph graph(network);
    RouteGuide guide(graph);
    const Point target{980, 545};                 // bottom-right, via (800,200) and (800,520)

    Point p{420, 175};
    guide.update(0.1f, p, target, true);
    const float first = guide.remaining();
    CHECK(guide.wrongTurns() == 0);

    // Drive east along the planned road for 6 s: the route is trimmed
    // (and refreshed for congestion), never counted as a wrong turn.
    for (int i = 0; i < 20; ++i) {
        p.x += 10.0f;
        guide.update(0.3f, p, target, true);
    }
    CHECK(guide.wrongTurns() == 0);
    CHECK_NEAR(guide.remaining(), first - 200.0f, 2.0f);
    CHECK(guide.route().front().x == p.x && guide.route().front().y == p.y);

    // Turn off down the x = 300 road... first jump to it: far from the route.
    p = {335, 400};
    guide.update(0.1f, p, target, true);
    CHECK(guide.wrongTurns() == 1);
    CHECK(guide.recalculating());

    // New call (new target): re-planned but not a wrong turn.
    guide.update(0.1f, p, {120, 545}, true);
    CHECK(guide.wrongTurns() == 1);
    for (int i = 0; i < 30; ++i) guide.update(0.1f, p, {120, 545}, true);
    CHECK(!guide.recalculating());

    // Driving the wrong way along the route also counts, once the grace
    // period for turning round at the start of a leg is over.
    RouteGuide back(graph);
    Point q{600, 165};
    back.update(0.1f, q, {980, 165}, true);        // straight east
    for (int i = 0; i < 25; ++i) {
        q.x -= 10.0f;                              // ...but going west for 7.5 s
        back.update(0.3f, q, {980, 165}, true);
        if (i == 10) CHECK(back.wrongTurns() == 0);   // still in the grace period
    }
    CHECK(back.wrongTurns() == 1);
    back.resetCount();
    CHECK(back.wrongTurns() == 0);
}

void distanceToPolyline() {
    const std::vector<Point> l = {{0, 0}, {100, 0}, {100, 100}};
    int seg = -1;
    CHECK_NEAR(RouteGuide::distanceTo(l, {50, 10}, &seg), 10.0f, 1e-4f);
    CHECK(seg == 0);
    CHECK_NEAR(RouteGuide::distanceTo(l, {130, 60}, &seg), 30.0f, 1e-4f);
    CHECK(seg == 1);
}

int main() {
    followsThenRecalculates();
    distanceToPolyline();
    sameRoadGoesStraight();
    turnsAtJunctions();
    picksTheShorterWay();
    routeEndsAtBothPoints();
    return check::failures();
}
