#include "Check.h"
#include "simulation/TrafficRules.h"
#include "simulation/VehicleType.h"

using namespace TrafficRules;

namespace {

void stoppingSpeedMatchesKinematics() {
    // v^2 = 2 b d  ->  b = 100, d = 50  ->  v = 100
    CHECK_NEAR(stoppingSpeed(50.0f, 100.0f), 100.0f, 1e-3f);
    CHECK_NEAR(brakingDistance(100.0f, 100.0f), 50.0f, 1e-3f);
    CHECK_NEAR(stoppingSpeed(-5.0f, 100.0f), 0.0f, 1e-6f);
}

void signalDecisions() {
    CHECK(mustStop(Signal::Red, 500.0f, 10.0f, 100.0f));
    CHECK(!mustStop(Signal::Green, 1.0f, 10.0f, 100.0f));
    // Yellow, far away and slow: a comfortable stop is possible -> stop.
    CHECK(mustStop(Signal::Yellow, 80.0f, 60.0f, 100.0f));    // needs 18 px
    // Yellow, too close to stop comfortably -> carry on through.
    CHECK(!mustStop(Signal::Yellow, 10.0f, 80.0f, 100.0f));   // needs 32 px
}

void followingKeepsDistance() {
    // Stopped leader right in front: must stop.
    CHECK_NEAR(followingSpeed(0.0f, 5.0f, 0.0f, 100.0f), 0.0f, 1e-6f);
    // Large gap: allowed speed grows with the gap.
    CHECK(followingSpeed(0.0f, 120.0f, 0.0f, 100.0f) >
          followingSpeed(0.0f, 60.0f, 0.0f, 100.0f));
    // A moving leader lets the follower go faster than a stopped one.
    CHECK(followingSpeed(50.0f, 40.0f, 30.0f, 100.0f) >
          followingSpeed(0.0f, 40.0f, 30.0f, 100.0f));
}

void corridorDetection() {
    const Point pos{0.0f, 0.0f};
    const Point east{1.0f, 0.0f};
    auto box = [](float cx, float cy, float hx, float hy) {
        return std::vector<Point>{{cx - hx, cy - hy}, {cx + hx, cy - hy},
                                  {cx + hx, cy + hy}, {cx - hx, cy + hy}};
    };

    // Car 60 px ahead in the same lane: hit, gap = 60 - 16 - 16 = 28.
    auto ahead = corridorCheck(pos, east, 16.0f, 9.0f, 140.0f, box(60, 0, 16, 9));
    CHECK(ahead.hit);
    CHECK_NEAR(ahead.gap, 28.0f, 1e-3f);

    // Same distance but in the opposite lane (50 px to the side): no hit.
    CHECK(!corridorCheck(pos, east, 16.0f, 9.0f, 140.0f, box(60, 50, 16, 9)).hit);

    // Behind us: no hit.
    CHECK(!corridorCheck(pos, east, 16.0f, 9.0f, 140.0f, box(-60, 0, 16, 9)).hit);

    // Beyond the lookahead: no hit.
    CHECK(!corridorCheck(pos, east, 16.0f, 9.0f, 140.0f, box(300, 0, 16, 9)).hit);

    // Rotated frame: heading south, obstacle below.
    const Point south{0.0f, 1.0f};
    CHECK(corridorCheck(pos, south, 16.0f, 9.0f, 140.0f, box(0, 70, 9, 16)).hit);
}

void vehicleMixFollowsWeights() {
    int counts[3] = {0, 0, 0};
    const int samples = 10000;
    for (int i = 0; i < samples; ++i) {
        const float u = (static_cast<float>(i) + 0.5f) / samples;
        ++counts[static_cast<int>(pickVehicleType(u))];
    }
    CHECK_NEAR(counts[0] / float(samples), 0.60f, 0.01f);   // cars
    CHECK_NEAR(counts[1] / float(samples), 0.25f, 0.01f);   // matatus
    CHECK_NEAR(counts[2] / float(samples), 0.15f, 0.01f);   // motorcycles

    // Matatus are the long, slow ones; motorcycles the narrow, quick ones.
    CHECK(specFor(VehicleType::Matatu).halfLength > specFor(VehicleType::Car).halfLength);
    CHECK(specFor(VehicleType::Motorcycle).halfWidth < specFor(VehicleType::Car).halfWidth);
    CHECK(specFor(VehicleType::Motorcycle).maxCruise > specFor(VehicleType::Matatu).maxCruise);
}

}

int main() {
    stoppingSpeedMatchesKinematics();
    signalDecisions();
    followingKeepsDistance();
    corridorDetection();
    vehicleMixFollowsWeights();
    return check::failures();
}
