#pragma once
#include <vector>
#include "Ambulance.h"

// Route-following "autopilot" for the headless evidence captures and the
// soak test, so a whole call (drive, stop, treat, hospital) can be
// recorded without a person at the keyboard. It produces the same
// DrivingInput as the keyboard, so it exercises the real driving model.
//
// Pure pursuit: aim at the point `lookahead` pixels further along the
// route, shifted into the left-hand lane; steer by the heading error.
// Speed: v = sqrt(2 * b * d) to stop on the target, a turn limit when the
// route bends within the next stretch, and a brake for anything in front.
// A target it overshot is reached by reversing; turning round in a
// narrow street is done as a three-point turn.
class DemoDriver {
public:
    static constexpr float Braking = 220.0f;      // px/s^2 the plan assumes
    static constexpr float CruiseSpeed = 230.0f;  // px/s
    static constexpr float TurnSpeed = 95.0f;     // px/s through a corner

    // `route` starts at the ambulance and ends at the target; `obstacles`
    // are the centres of nearby vehicles and people.
    DrivingInput drive(const Ambulance& ambulance, const std::vector<Point>& route,
                       const std::vector<Point>& obstacles, bool stopAtEnd, float deltaTime);

    // Point `distance` along the polyline (clamped to its ends) and the
    // direction of the segment it lies on.
    static Point along(const std::vector<Point>& route, float distance, Point* direction = nullptr);

private:
    float waited_{0.0f};       // seconds spent waiting behind something
    float stalled_{0.0f};      // seconds pushing forward without moving
    float reversing_{0.0f};    // seconds of reversing left (three-point turn)
    float reverseSteer_{0.0f};
};
