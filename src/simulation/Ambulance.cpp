#include "Ambulance.h"
#include "../core/Color.h"
#include "../shapes/CircleEllipse.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float PI = 3.14159265358979323846f;
constexpr float MaxSteer = 30.0f * PI / 180.0f;
constexpr float WheelRadius = 6.0f;
}

Ambulance::Ambulance(Point startPosition) : position_(startPosition) {
    buildModel();
    syncModel();
}

void Ambulance::buildModel() {
    root_ = makeGroup("root", Mat3::identity());

    // Wheels first so the body is drawn over their inner half.
    const Point mounts[4] = {{-18, -18}, {-18, 18}, {18, -18}, {18, 18}};
    for (int i = 0; i < 4; ++i) {
        auto mount = makeGroup("wheel-mount", Mat3::translation(mounts[i].x, mounts[i].y));
        SceneNode* parent = root_->add(std::move(mount));
        if (i >= 2) {
            // Front wheels get an extra steering node between mount and wheel.
            parent = parent->add(makeGroup(i == 2 ? "steer-left" : "steer-right",
                                           Mat3::identity()));
            (i == 2 ? steerLeft_ : steerRight_) = parent;
        }
        wheels_[i] = parent->add(makeWheel("wheel", WheelRadius, 5.0f));
    }

    root_->add(makeRect("body", -30, -16, 30, 16,
                        sf::Color(225, 225, 225), Colors::Ambulance, 2.0f));
    root_->add(makeRect("cab", 5, -13, 27, 13,
                        sf::Color(160, 45, 45), Colors::AmbulanceAccent, 1.0f));
    root_->add(makeRect("windscreen", 21, -11, 25, 11, sf::Color(60, 80, 100)));

    // Roof cross made of two bars.
    root_->add(makeRect("cross-h", -18, -2, -4, 2, Colors::AmbulanceAccent));
    root_->add(makeRect("cross-v", -13, -7, -9, 7, Colors::AmbulanceAccent));

    SceneNode* lightBar = root_->add(makeGroup("light-bar", Mat3::translation(1, 0)));
    lightBar->add(makeRect("housing", -2, -13, 2, 13, sf::Color(40, 40, 44)));
    redLight_ = lightBar->add(makeDisc("light-red", {0, -8}, 3, sf::Color::Red));
    blueLight_ = lightBar->add(makeDisc("light-blue", {0, 8}, 3, sf::Color::Blue));

    // Rotating beacon: a beam polygon spun by a time-driven rotation matrix.
    beacon_ = root_->add(makeGroup("beacon", Mat3::translation(-24, 0)));
    auto beam = std::make_unique<SceneNode>("beam", SceneNode::Kind::Polygon);
    beam->shape = {{0, 0}, {11, -4}, {11, 4}};
    beam->fill = sf::Color(120, 170, 255, 170);
    beacon_->add(std::move(beam));
    beacon_->add(makeDisc("beacon-core", {0, 0}, 2, sf::Color(200, 220, 255)));
}

void Ambulance::syncModel() {
    // world = T(position) * R(heading): rotate in local space, then place.
    root_->local = Mat3::translation(position_.x, position_.y) *
                   Mat3::rotationDegrees(headingDegrees_);

    steerLeft_->local = Mat3::rotation(steer_);
    steerRight_->local = Mat3::rotation(steer_);
    for (SceneNode* wheel : wheels_)
        wheel->spin = wheelSpin_;

    // Siren lights alternate four times a second.
    const bool phase = std::fmod(time_, 0.5f) < 0.25f;
    redLight_->fill = phase ? sf::Color(255, 60, 60) : sf::Color(90, 25, 25);
    blueLight_->fill = phase ? sf::Color(25, 35, 90) : sf::Color(70, 120, 255);

    beacon_->children.front()->local = Mat3::rotation(time_ * 6.0f);
}

void Ambulance::move(float distance) {
    const float radians = headingDegrees_ * PI / 180.0f;
    position_.x += std::cos(radians) * distance;
    position_.y += std::sin(radians) * distance;

    // Rolling without slipping: angle = arc length / radius.
    wheelSpin_ += distance / WheelRadius;
}

void Ambulance::turn(float degrees) {
    headingDegrees_ += degrees;
    if (headingDegrees_ >= 360.0f) headingDegrees_ -= 360.0f;
    if (headingDegrees_ < 0.0f) headingDegrees_ += 360.0f;
}

void Ambulance::handleInput(float deltaTime) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        move(moveSpeed_ * deltaTime);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        move(-moveSpeed_ * deltaTime);

    float steerInput = 0.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        turn(-turnSpeed_ * deltaTime);
        steerInput -= 1.0f;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        turn(turnSpeed_ * deltaTime);
        steerInput += 1.0f;
    }

    // Front wheels swing towards the input, clamped to +-30 degrees.
    const float target = steerInput * MaxSteer;
    const float rate = 4.0f * deltaTime;
    steer_ += std::clamp(target - steer_, -rate, rate);
}

void Ambulance::update(float deltaTime) {
    time_ += deltaTime;
    handleInput(deltaTime);
    syncModel();
}

void Ambulance::draw(sf::RenderTarget& target,
                     const LineRenderer& lines,
                     bool debug) const {
    root_->draw(target, lines, Mat3::identity(), debug);

    if (debug) {
        ShapeAlgorithms::drawPoints(
            target, ShapeAlgorithms::filledCircle(position_, 3),
            Colors::Debug, 1.0f);
        const Point headingEnd = root_->local.apply(Point{50, 0});
        lines.draw(target, position_, headingEnd, Colors::Debug, 2.0f);
    }
}

Point Ambulance::position() const { return position_; }
float Ambulance::heading() const { return headingDegrees_; }
float Ambulance::steerAngle() const { return steer_; }
