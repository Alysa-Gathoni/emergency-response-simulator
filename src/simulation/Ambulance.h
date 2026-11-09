#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "SceneNode.h"
#include "../lines/LineRenderer.h"

// The ambulance is a hierarchical model:
//
//   root   T(position) * R(heading)
//   |-- wheels: rear-left, rear-right            (fixed mounts)
//   |-- front-left / front-right mount -> steer R(steerAngle) -> wheel
//   |-- body, cab, roof cross
//   |-- light bar -> red light, blue light       (alternate flashing)
//   |-- beacon T(roof) * R(time)                  (rotating beam)
//   `-- brake lights                              (lit while braking)
//
// Driving uses a simplified bicycle model: the heading changes at
//     dHeading/dt = speed / wheelbase * tan(steerAngle)
// and the steering lock itself shrinks as speed rises, so the turning
// radius grows with speed and the ambulance cannot spin on the spot.
// With no steering input the heading is eased onto the nearest road
// direction (small offsets only), which keeps it straight in its lane.
struct DrivingInput {
    float throttle{0.0f};   // -1 (brake / reverse) .. 1 (accelerate)
    float steer{0.0f};      // -1 (left) .. 1 (right)
    bool handbrake{false};
};

class Ambulance {
public:
    explicit Ambulance(Point startPosition);

    void update(float deltaTime);

    // Reads the keyboard (W/S or arrows, A/D, Space) into a DrivingInput.
    static DrivingInput readKeyboard();
    void drive(const DrivingInput& input, float deltaTime);

    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool debug) const;

    Point position() const;
    float heading() const;          // degrees, 0 = east
    float steerAngle() const;       // radians, + = right
    float speed() const;            // px/s along the heading, < 0 reversing
    bool braking() const;

    // Largest steering angle (radians) allowed at `speed` px/s.
    static float maxSteerAt(float speed);
    static float topSpeed();

    // Corners of the body in world space (for collisions and traffic).
    std::vector<Point> footprint() const;

    // Collision response: push out of the obstacle along `separation`,
    // bounce back slightly and lose most of the speed.
    void resolveCollision(Point separation);

    // Place the ambulance (mission restart).
    void resetTo(Point position, float headingDegrees);

private:
    std::unique_ptr<SceneNode> root_;
    SceneNode* steerLeft_{};
    SceneNode* steerRight_{};
    SceneNode* wheels_[4]{};
    SceneNode* redLight_{};
    SceneNode* blueLight_{};
    SceneNode* beacon_{};
    SceneNode* brakeLights_[2]{};

    Point position_;
    float headingDegrees_{0.0f};
    float steer_{0.0f};
    float wheelSpin_{0.0f};
    float time_{0.0f};
    float speed_{0.0f};
    bool braking_{false};

    void buildModel();
    void syncModel();
};
