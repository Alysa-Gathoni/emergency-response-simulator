#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "SceneNode.h"
#include "../lines/LineRenderer.h"

// The ambulance is a hierarchical model:
//
//   root   T(position) * R(heading)
//   |-- wheels: rear-left, rear-right            (fixed mounts)
//   |-- front-left / front-right mount -> steer R(steerAngle) -> wheel
//   |-- body, cab, roof cross
//   |-- light bar -> red light, blue light       (alternate flashing)
//   `-- beacon T(roof) * R(time)                  (rotating beam)
class Ambulance {
public:
    explicit Ambulance(Point startPosition);

    void update(float deltaTime);
    void handleInput(float deltaTime);

    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool debug) const;

    Point position() const;
    float heading() const;          // degrees, 0 = east
    float steerAngle() const;       // radians, + = right

private:
    std::unique_ptr<SceneNode> root_;
    SceneNode* steerLeft_{};
    SceneNode* steerRight_{};
    SceneNode* wheels_[4]{};
    SceneNode* redLight_{};
    SceneNode* blueLight_{};
    SceneNode* beacon_{};

    Point position_;
    float headingDegrees_{0.0f};
    float steer_{0.0f};
    float wheelSpin_{0.0f};
    float time_{0.0f};

    float moveSpeed_{150.0f};
    float turnSpeed_{120.0f};

    void buildModel();
    void move(float distance);
    void turn(float degrees);
    void syncModel();
};
