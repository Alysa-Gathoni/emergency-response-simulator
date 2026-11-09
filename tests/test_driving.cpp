#include "Check.h"
#include "simulation/Ambulance.h"
#include <cmath>

namespace {

constexpr float Dt = 1.0f / 60.0f;

void run(Ambulance& a, DrivingInput in, float seconds) {
    for (int i = 0; i < static_cast<int>(seconds / Dt); ++i)
        a.drive(in, Dt);
}

void topSpeedIsCapped() {
    Ambulance a({500, 360});
    run(a, {1.0f, 0.0f, false}, 6.0f);
    CHECK_NEAR(a.speed(), Ambulance::topSpeed(), 0.5f);
}

void steeringFadesWithSpeed() {
    CHECK(Ambulance::maxSteerAt(0.0f) > Ambulance::maxSteerAt(60.0f));
    CHECK(Ambulance::maxSteerAt(60.0f) > Ambulance::maxSteerAt(Ambulance::topSpeed()));
    CHECK_NEAR(Ambulance::maxSteerAt(-50.0f), Ambulance::maxSteerAt(50.0f), 1e-6f);

    // Turning radius L / tan(steer) at top speed is clearly wider than
    // at junction speed, but still fits a turn across the 2x2 grid.
    const float slow = 36.0f / std::tan(Ambulance::maxSteerAt(40.0f));
    const float fast = 36.0f / std::tan(Ambulance::maxSteerAt(Ambulance::topSpeed()));
    CHECK(fast > slow * 1.5f);
    CHECK(fast < 250.0f);
}

void headingAssistStraightens() {
    // A small offset from a road direction is eased away when not steering.
    Ambulance a({500, 360});
    a.resetTo({500, 360}, 8.0f);
    run(a, {1.0f, 0.0f, false}, 2.0f);
    CHECK_NEAR(a.heading(), 0.0f, 0.5f);

    a.resetTo({500, 360}, 352.0f);
    run(a, {1.0f, 0.0f, false}, 2.0f);
    const float h = a.heading();
    CHECK(h < 0.5f || h > 359.5f);

    // A deliberate diagonal is left alone.
    a.resetTo({500, 360}, 45.0f);
    run(a, {1.0f, 0.0f, false}, 1.0f);
    CHECK_NEAR(a.heading(), 45.0f, 0.01f);
}

void assistNeverFightsTheDriver() {
    // While steering, the heading keeps turning through the road angle.
    Ambulance a({500, 360});
    a.resetTo({500, 360}, 0.0f);
    run(a, {1.0f, 0.0f, false}, 1.0f);
    run(a, {1.0f, 1.0f, false}, 0.5f);
    CHECK(a.heading() > 5.0f && a.heading() < 90.0f);
}

}

int main() {
    topSpeedIsCapped();
    steeringFadesWithSpeed();
    headingAssistStraightens();
    assistNeverFightsTheDriver();
    return check::failures();
}
