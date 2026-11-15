#include "Check.h"
#include "simulation/Ambulance.h"
#include "simulation/SkidMarks.h"
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

void headOnBouncesBack() {
    Ambulance a({500, 360});
    a.resetTo({500, 360}, 0.0f);                 // facing +x
    run(a, {1.0f, 0.0f, false}, 1.0f);
    const float before = a.speed();
    a.resolveCollision({-2.0f, 0.0f});           // wall ahead pushes back along -x
    CHECK_NEAR(a.speed(), -0.3f * before, 0.5f);
    CHECK(a.impact() > 0.0f);
}

void glancingHitSlides() {
    Ambulance a({500, 360});
    a.resetTo({500, 360}, 20.0f);                // 20 degrees towards a wall below
    run(a, {1.0f, 0.0f, false}, 1.0f);
    const float before = a.speed();
    a.resolveCollision({0.0f, -2.0f});           // wall below pushes up
    CHECK(a.speed() > 0.7f * before);            // keeps most of its speed
    CHECK(a.heading() < 20.0f);                  // nose turned along the wall
}

void movingAwayIsUntouched() {
    Ambulance a({500, 360});
    a.resetTo({500, 360}, 180.0f);               // driving away from a wall on the right
    run(a, {1.0f, 0.0f, false}, 1.0f);
    const float before = a.speed();
    a.resolveCollision({-1.0f, 0.0f});
    CHECK_NEAR(a.speed(), before, 1e-3f);
}

}

void handbrakeLeavesSkidMarks() {
    Ambulance a({200, 200});
    SkidMarks marks;
    DrivingInput go;
    go.throttle = 1.0f;
    for (int i = 0; i < 120; ++i) { a.update(1.0f / 60.0f, go); marks.update(1.0f / 60.0f, a); }
    CHECK(a.skid() == 0.0f);                    // straight acceleration: no slide
    CHECK(marks.marks().empty());

    DrivingInput slide;
    slide.handbrake = true;
    for (int i = 0; i < 20; ++i) { a.update(1.0f / 60.0f, slide); marks.update(1.0f / 60.0f, a); }
    CHECK(!marks.marks().empty());
    // Two marks (one per rear wheel) per frame, behind the ambulance.
    const auto& m = marks.marks().back();
    CHECK(m.b.x < a.position().x);
    CHECK(m.strength > 0.0f && m.strength <= 1.0f);

    // Marks fade out and are removed after their lifetime.
    DrivingInput none;
    for (int i = 0; i < static_cast<int>((SkidMarks::Lifetime + 0.5f) * 60); ++i) {
        a.update(1.0f / 60.0f, none);
        marks.update(1.0f / 60.0f, a);
    }
    CHECK(marks.marks().empty());
}

void sirenLightsAlternate() {
    Ambulance a({100, 100});
    CHECK(a.sirenOn());
    const int first = a.sirenPhase();
    a.update(0.25f, DrivingInput{});
    CHECK(a.sirenPhase() != first);
    // Lights sit on the roof either side of the centre line.
    CHECK_NEAR(a.redLightPosition().y, 92.0f, 1e-3f);
    CHECK_NEAR(a.blueLightPosition().y, 108.0f, 1e-3f);
    a.setSiren(false);
    CHECK(!a.sirenOn());
}

int main() {
    handbrakeLeavesSkidMarks();
    sirenLightsAlternate();
    topSpeedIsCapped();
    steeringFadesWithSpeed();
    headingAssistStraightens();
    assistNeverFightsTheDriver();
    headOnBouncesBack();
    glancingHitSlides();
    movingAwayIsUntouched();
    return check::failures();
}
