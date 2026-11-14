#include "Ambulance.h"
#include "../core/Color.h"
#include "../shapes/CircleEllipse.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float PI = 3.14159265358979323846f;
constexpr float MaxSteer = 30.0f * PI / 180.0f;
constexpr float WheelRadius = 6.0f;

// Driving model parameters (pixels, seconds).
constexpr float Wheelbase = 36.0f;          // front to rear axle
constexpr float MaxForward = 140.0f;         // ~100 km/h on the HUD scale
constexpr float MaxReverse = -60.0f;
constexpr float Acceleration = 115.0f;
constexpr float BrakeDecel = 320.0f;
constexpr float HandbrakeDecel = 560.0f;
constexpr float RollingDrag = 60.0f;         // coasting slow-down
constexpr float SteerRate = 3.5f;            // rad/s the wheels can turn
constexpr float SteerReturnRate = 6.0f;      // faster self-centring on release

// Speed-sensitive steering: full lock is reduced as speed rises,
//     maxSteer(v) = MaxSteer / (1 + |v| / SteerFade)
// so the turning radius L / tan(maxSteer) grows with speed instead of
// staying at its tightest value (which made high speed twitchy).
constexpr float SteerFade = 110.0f;

// Heading assist: with no steering input, a heading within AssistWindow
// of a road direction (0, 90, 180, 270 degrees) is eased onto it, so the
// ambulance straightens up after a turn instead of drifting into kerbs.
constexpr float AssistWindow = 14.0f;        // degrees
constexpr float AssistRate = 40.0f;          // degrees per second at speed

// Collision response.
constexpr float Restitution = 0.3f;          // share of the normal speed bounced back
constexpr float Friction = 0.15f;            // share of the sliding speed lost
constexpr float YawKick = 0.35f;             // how far a glancing hit turns the nose
constexpr float ImpactDecay = 3.0f;          // impact feedback fades in 1/3 s
}

float Ambulance::maxSteerAt(float speed) {
    return MaxSteer / (1.0f + std::abs(speed) / SteerFade);
}

float Ambulance::topSpeed() { return MaxForward; }

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
                        Rgba(225, 225, 225), Colors::Ambulance, 2.0f));
    root_->add(makeRect("cab", 5, -13, 27, 13,
                        Rgba(160, 45, 45), Colors::AmbulanceAccent, 1.0f));
    root_->add(makeRect("windscreen", 21, -11, 25, 11, Rgba(60, 80, 100)));

    // Roof cross made of two bars.
    root_->add(makeRect("cross-h", -18, -2, -4, 2, Colors::AmbulanceAccent));
    root_->add(makeRect("cross-v", -13, -7, -9, 7, Colors::AmbulanceAccent));

    SceneNode* lightBar = root_->add(makeGroup("light-bar", Mat3::translation(1, 0)));
    lightBar->add(makeRect("housing", -2, -13, 2, 13, Rgba(40, 40, 44)));
    redLight_ = lightBar->add(makeDisc("light-red", {0, -8}, 3, Rgba::Red));
    blueLight_ = lightBar->add(makeDisc("light-blue", {0, 8}, 3, Rgba::Blue));

    // Rotating beacon: a beam polygon spun by a time-driven rotation matrix.
    beacon_ = root_->add(makeGroup("beacon", Mat3::translation(-24, 0)));
    auto beam = std::make_unique<SceneNode>("beam", SceneNode::Kind::Polygon);
    beam->shape = {{0, 0}, {11, -4}, {11, 4}};
    beam->fill = Rgba(120, 170, 255, 170);
    beacon_->add(std::move(beam));
    beacon_->add(makeDisc("beacon-core", {0, 0}, 2, Rgba(200, 220, 255)));

    brakeLights_[0] = root_->add(makeRect("brake-left", -31, -14, -28, -9, Rgba(90, 20, 20)));
    brakeLights_[1] = root_->add(makeRect("brake-right", -31, 9, -28, 14, Rgba(90, 20, 20)));
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
    redLight_->fill = phase ? Rgba(255, 60, 60) : Rgba(90, 25, 25);
    blueLight_->fill = phase ? Rgba(25, 35, 90) : Rgba(70, 120, 255);

    beacon_->children.front()->local = Mat3::rotation(time_ * 6.0f);

    for (SceneNode* light : brakeLights_)
        light->fill = braking_ ? Rgba(255, 50, 40) : Rgba(90, 20, 20);
}

void Ambulance::drive(const DrivingInput& input, float deltaTime) {
    // 1. Longitudinal speed: accelerate, brake, reverse, coast.
    braking_ = false;
    if (input.handbrake) {
        const float drop = HandbrakeDecel * deltaTime;
        speed_ = std::abs(speed_) <= drop ? 0.0f : speed_ - std::copysign(drop, speed_);
        braking_ = true;
    } else if (input.throttle > 0.0f) {
        if (speed_ < 0.0f) { speed_ += BrakeDecel * deltaTime; braking_ = true; }
        else speed_ += Acceleration * input.throttle * deltaTime;
    } else if (input.throttle < 0.0f) {
        // S brakes while rolling forwards, then engages reverse.
        if (speed_ > 0.0f) { speed_ -= BrakeDecel * deltaTime; braking_ = true; }
        else speed_ += Acceleration * 0.6f * input.throttle * deltaTime;
    } else {
        const float drop = RollingDrag * deltaTime;
        speed_ = std::abs(speed_) <= drop ? 0.0f : speed_ - std::copysign(drop, speed_);
    }
    speed_ = std::clamp(speed_, MaxReverse, MaxForward);

    // 2. Steering: the wheels move towards the input at a limited rate.
    const float steerInput = std::clamp(input.steer, -1.0f, 1.0f);
    const float limit = maxSteerAt(speed_);
    const float target = steerInput * limit;
    const float rate = (steerInput == 0.0f ? SteerReturnRate : SteerRate) * deltaTime;
    steer_ += std::clamp(target - steer_, -rate, rate);
    steer_ = std::clamp(steer_, -limit, limit);

    // 3. Bicycle model: heading rate = v / L * tan(steer).
    const float headingRate = speed_ / Wheelbase * std::tan(steer_);
    headingDegrees_ += headingRate * deltaTime * 180.0f / PI;

    if (steerInput == 0.0f && std::abs(speed_) > 10.0f) {
        const float road = std::round(headingDegrees_ / 90.0f) * 90.0f;
        const float offset = road - headingDegrees_;
        if (std::abs(offset) < AssistWindow) {
            const float step = AssistRate * std::min(1.0f, std::abs(speed_) / 60.0f) * deltaTime;
            headingDegrees_ += std::clamp(offset, -step, step);
        }
    }
    headingDegrees_ = std::fmod(headingDegrees_ + 360.0f, 360.0f);

    // 4. Move along the new heading; wheels roll by distance / radius.
    const float radians = headingDegrees_ * PI / 180.0f;
    const float distance = speed_ * deltaTime;
    position_.x += std::cos(radians) * distance;
    position_.y += std::sin(radians) * distance;
    wheelSpin_ += distance / WheelRadius;
}

void Ambulance::update(float deltaTime, const DrivingInput& input) {
    time_ += deltaTime;
    impact_ = std::max(0.0f, impact_ - ImpactDecay * deltaTime);
    drive(input, deltaTime);
    syncModel();
}

void Ambulance::resolveCollision(Point separation) {
    position_.x += separation.x;
    position_.y += separation.y;

    const float length = std::hypot(separation.x, separation.y);
    if (length < 1e-4f) { syncModel(); return; }

    // Contact normal n points out of the obstacle. Split the velocity into
    // a normal part (into the obstacle) and a tangential part (along it):
    //     v_n = (v . n) n,     v_t = v - v_n
    //     v'  = (1 - Friction) v_t - Restitution v_n
    // Only an approaching hit (v . n < 0) is changed, so resting contact
    // does not keep bouncing.
    const Point n{separation.x / length, separation.y / length};
    const float a = headingDegrees_ * PI / 180.0f;
    const Point forward{std::cos(a), std::sin(a)};
    const Point v{forward.x * speed_, forward.y * speed_};
    const float vn = v.x * n.x + v.y * n.y;

    if (vn < 0.0f) {
        const Point normal{n.x * vn, n.y * vn};
        const Point tangent{v.x - normal.x, v.y - normal.y};
        const Point after{tangent.x * (1.0f - Friction) - normal.x * Restitution,
                          tangent.y * (1.0f - Friction) - normal.y * Restitution};

        // The bicycle model only moves along the heading, so keep the
        // part of the new velocity along it.
        speed_ = after.x * forward.x + after.y * forward.y;

        // Glancing hit: the nose is pushed round to follow the obstacle.
        const float along = std::hypot(tangent.x, tangent.y);
        if (along > 1.0f) {
            float turn = std::atan2(tangent.y, tangent.x) - a;
            turn = std::atan2(std::sin(turn), std::cos(turn));      // wrap to -pi..pi
            if (std::abs(turn) < PI / 3.0f)
                headingDegrees_ += turn * YawKick * 180.0f / PI;
        }

        // Feedback: impact strength (sparks, later camera shake) and where.
        impact_ = std::max(impact_, std::clamp(-vn / 120.0f, 0.2f, 1.0f));
        contact_ = {position_.x - n.x * 18.0f, position_.y - n.y * 18.0f};
    }
    syncModel();
}

float Ambulance::impact() const { return impact_; }

void Ambulance::resetTo(Point position, float headingDegrees) {
    position_ = position;
    headingDegrees_ = headingDegrees;
    speed_ = 0.0f;
    steer_ = 0.0f;
    syncModel();
}

std::vector<Point> Ambulance::footprint() const {
    return root_->local.apply(std::vector<Point>{
        {-30, -16}, {30, -16}, {30, 16}, {-30, 16}});
}

void Ambulance::draw(IRenderer& target,
                     const LineRenderer& lines,
                     bool debug) const {
    root_->draw(target, lines, Mat3::identity(), debug);

    // Sparks at the last contact point, fading with the impact strength.
    if (impact_ > 0.05f) {
        const Rgba spark(255, 220, 90, static_cast<int>(255 * std::min(1.0f, impact_ * 1.5f)));
        for (int k = 0; k < 8; ++k) {
            const float ang = k * PI / 4.0f + time_ * 7.0f;
            const float reach = 6.0f + 14.0f * impact_;
            lines.draw(target, contact_,
                       {contact_.x + std::cos(ang) * reach, contact_.y + std::sin(ang) * reach},
                       spark, 1.0f);
        }
    }

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
float Ambulance::speed() const { return speed_; }
bool Ambulance::braking() const { return braking_; }
