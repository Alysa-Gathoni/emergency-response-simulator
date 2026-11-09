#include "Check.h"
#include "city/RoadNetwork.h"
#include "city/SignalController.h"
#include <cmath>

namespace {

void conflictingGroupsNeverMoveTogether() {
    SignalController c;
    for (int step = 0; step < 2000; ++step) {
        const Signal ns = c.signalFor(SignalGroup::NorthSouth);
        const Signal ew = c.signalFor(SignalGroup::EastWest);
        CHECK(ns == Signal::Red || ew == Signal::Red);
        c.update(0.05f);
    }
}

void phaseSequence() {
    SignalTiming t{8.0f, 2.5f, 1.5f};
    SignalController c(0.0f, t);
    CHECK(c.signalFor(SignalGroup::NorthSouth) == Signal::Green);

    c.update(8.1f);
    CHECK(c.signalFor(SignalGroup::NorthSouth) == Signal::Yellow);

    c.update(2.5f);   // all-red clearance
    CHECK(c.signalFor(SignalGroup::NorthSouth) == Signal::Red);
    CHECK(c.signalFor(SignalGroup::EastWest) == Signal::Red);

    c.update(1.5f);
    CHECK(c.signalFor(SignalGroup::EastWest) == Signal::Green);
    CHECK_NEAR(c.cycleLength(), 24.0f, 1e-4f);
}

void offsetMatchesSimulatedTime() {
    SignalController shifted(9.0f);
    SignalController simulated;
    simulated.update(9.0f);
    CHECK(shifted.phase() == simulated.phase());
    CHECK_NEAR(shifted.timeInPhase(), simulated.timeInPhase(), 1e-3f);
}

void longFrameSkipsWholePhases() {
    SignalController c;
    c.update(c.cycleLength() + 0.5f);   // one full cycle later
    CHECK(c.phase() == SignalController::Phase::NorthSouthGreen);
}

void stopLinesSitOnTheLeftLane() {
    RoadNetwork net;
    CHECK(net.intersections().size() == 4);
    CHECK(net.stopLines().size() == 16);

    for (const StopLine& s : net.stopLines()) {
        const Point c = net.intersections()[s.intersection].centre;
        const Point d = headingVector(s.approach);
        // Stop line is upstream of the junction ...
        const float along = (s.a.x - c.x) * d.x + (s.a.y - c.y) * d.y;
        CHECK_NEAR(along, -RoadNetwork::StopLineDistance, 1e-3f);
        // ... and covers the lane on the driver's left (left-hand traffic).
        const Point lane = RoadNetwork::laneCentre(s.a, d);
        const float t = ((lane.x - s.a.x) * (s.b.x - s.a.x) +
                         (lane.y - s.a.y) * (s.b.y - s.a.y)) /
                        (RoadNetwork::HalfWidth * RoadNetwork::HalfWidth);
        CHECK(t > 0.0f && t < 1.0f);
    }
}

void routeIsClosedAndInLane() {
    // Anticlockwise loop round the grid: four tight left turns.
    const SplinePath loop = RoadNetwork::buildRoute(
        {{300, 520}, {800, 520}, {800, 200}, {300, 200}});

    const Point start = loop.pointAtDistance(0.0f);
    const Point end = loop.pointAtDistance(loop.length() - 0.01f);
    CHECK(std::hypot(start.x - end.x, start.y - end.y) < 1.0f);

    // Halfway along the bottom street, eastbound traffic is in the lane
    // north of the centre line (y = 520 - LaneOffset).
    bool found = false;
    for (float s = 0.0f; s < loop.length(); s += 2.0f) {
        const Point p = loop.pointAtDistance(s);
        if (std::fabs(p.x - 550.0f) < 1.5f && p.y > 400.0f) {
            CHECK_NEAR(p.y, 520.0f - RoadNetwork::LaneOffset, 0.5f);
            CHECK(loop.directionAtDistance(s).x > 0.99f);
            found = true;
        }
    }
    CHECK(found);
}

}

int main() {
    conflictingGroupsNeverMoveTogether();
    phaseSequence();
    offsetMatchesSimulatedTime();
    longFrameSkipsWholePhases();
    stopLinesSitOnTheLeftLane();
    routeIsClosedAndInLane();
    return check::failures();
}
