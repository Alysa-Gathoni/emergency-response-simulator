#include "Mission.h"
#include "../shapes/CircleEllipse.h"
#include "../core/Color.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float MinSpawnDistance = 400.0f;   // never right next to the ambulance
constexpr float MaxSpawnDistance = 1500.0f;  // and reachable within the time limit
constexpr float ResultPause = 2.5f;

const Rgba HospitalColour(90, 170, 255);

// Arc of the midpoint circle from 12 o'clock, clockwise, `progress` of the way.
std::vector<Point> progressArc(Point c, int radius, float progress) {
    const float limit = progress * 2.0f * PI;
    std::vector<Point> arc;
    for (const Point& p : ShapeAlgorithms::midpointCircle(c, radius)) {
        float a = std::atan2(p.x - c.x, -(p.y - c.y));
        if (a < 0.0f) a += 2.0f * PI;
        if (a <= limit) arc.push_back(p);
    }
    return arc;
}

}

Mission::Mission(std::vector<Point> spawnPoints, Point hospital, unsigned seed)
    : spawnPoints_(std::move(spawnPoints)), hospital_(hospital), rng_(seed),
      marker_({0.0f, 0.0f}), hospitalMarker_(hospital) {
    hospitalMarker_.setColour(HospitalColour);
    nextIncident(hospital);
}

void Mission::nextIncident(Point ambulancePosition) {
    std::uniform_int_distribution<int> pick(0, static_cast<int>(spawnPoints_.size()) - 1);
    std::uniform_real_distribution<float> kind(0.0f, 1.0f);

    // Different from the last one and not too close to the ambulance.
    int candidate = pick(rng_);
    for (int tries = 0; tries < 50; ++tries) {
        const Point p = spawnPoints_[candidate];
        const float d = std::hypot(p.x - ambulancePosition.x, p.y - ambulancePosition.y);
        if (candidate != currentSpawn_ && d >= MinSpawnDistance && d <= MaxSpawnDistance) break;
        candidate = pick(rng_);
    }

    currentSpawn_ = candidate;
    type_ = pickIncident(kind(rng_));
    marker_.setPosition(spawnPoints_[candidate]);
    marker_.setColour(incidentInfo(type_).colour);
    state_ = State::EnRoute;
    elapsed_ = 0.0f;
    firstLeg_ = 0.0f;
    legLimit_ = incidentInfo(type_).timeLimit;
    hold_ = 0.0f;
    treatment_ = 0.0f;
}

void Mission::restart(Point ambulancePosition) {
    rng_.seed(3);
    currentSpawn_ = -1;
    completed_ = 0;
    missed_ = 0;
    lastResponse_ = 0.0f;
    nextIncident(ambulancePosition);
}

void Mission::complete() {
    state_ = State::Completed;
    ++completed_;
    lastResponse_ = drivingTime();
    pause_ = ResultPause;
}

void Mission::miss() {
    state_ = State::Missed;
    ++missed_;
    pause_ = ResultPause;
}

void Mission::update(float deltaTime, Point ambulancePosition, float ambulanceSpeed) {
    marker_.update(deltaTime);
    hospitalMarker_.update(deltaTime);
    const Point goal = target();
    distance_ = std::hypot(goal.x - ambulancePosition.x, goal.y - ambulancePosition.y);
    const float radius = state_ == State::Transport ? HospitalRadius : ArrivalRadius;
    const bool stopped = distance_ < radius && std::abs(ambulanceSpeed) < StopSpeed;

    switch (state_) {
        case State::Completed:
        case State::Missed:
            pause_ -= deltaTime;
            if (pause_ <= 0.0f) nextIncident(ambulancePosition);
            return;

        case State::EnRoute:
            elapsed_ += deltaTime;
            hold_ = stopped ? hold_ + deltaTime : 0.0f;
            if (hold_ >= HoldTime) {
                state_ = State::OnScene;
                treatment_ = 0.0f;
                hold_ = 0.0f;
            } else if (elapsed_ > legLimit_) {
                miss();
            }
            return;

        case State::OnScene:
            // The leg timer stops while the crew works; moving off the scene
            // abandons the treatment and the drive there continues.
            if (!stopped) {
                state_ = State::EnRoute;
                treatment_ = 0.0f;
                return;
            }
            treatment_ += deltaTime;
            if (treatment_ < treatmentTime()) return;
            if (!needsHospital(type_)) {
                firstLeg_ = elapsed_;
                complete();
                return;
            }
            // Second leg: patient on board, new timer to the hospital.
            firstLeg_ = elapsed_;
            elapsed_ = 0.0f;
            legLimit_ = (std::abs(hospital_.x - ambulancePosition.x) +
                         std::abs(hospital_.y - ambulancePosition.y)) / TransportSpeed
                      + TransportAllowance;
            state_ = State::Transport;
            return;

        case State::Transport:
            elapsed_ += deltaTime;
            hold_ = stopped ? hold_ + deltaTime : 0.0f;
            if (hold_ >= HoldTime) complete();
            else if (elapsed_ > legLimit_) miss();
            return;
    }
}

void Mission::draw(IRenderer& target, const LineRenderer&, bool debug) const {
    const Point c = marker_.position();
    const Point h = hospital_;

    if (state_ == State::EnRoute || state_ == State::OnScene) marker_.draw(target, debug);
    if (state_ == State::Transport) hospitalMarker_.draw(target, debug);

    // Hold-still indicator: keep only the midpoint-circle pixels whose
    // angle (measured clockwise from 12 o'clock) is within progress * 2pi.
    const Point goal = state_ == State::Transport ? h : c;
    if (driving() && hold_ > 0.0f)
        ShapeAlgorithms::drawPoints(target, progressArc(goal, 38, holdProgress()),
                                    Rgba(90, 230, 120), 3.0f);

    // Treatment: a second, wider ring fills up while the crew works.
    if (state_ == State::OnScene) {
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::midpointCircle(c, 46),
                                    Rgba(40, 50, 60), 3.0f);
        ShapeAlgorithms::drawPoints(target, progressArc(c, 46, treatmentProgress()),
                                    Rgba(120, 200, 255), 5.0f);
    }

    if (state_ == State::Completed) {
        const Point at = needsHospital(type_) ? h : c;
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::midpointCircle(at, 30),
                                    Rgba(90, 230, 120), 4.0f);
    }

    if (debug && driving()) {
        ShapeAlgorithms::drawPoints(
            target, ShapeAlgorithms::midpointCircle(
                        goal, static_cast<int>(state_ == State::Transport ? HospitalRadius : ArrivalRadius)),
            Colors::Debug, 1.0f);
    }
}

Mission::State Mission::state() const { return state_; }
bool Mission::driving() const { return state_ == State::EnRoute || state_ == State::Transport; }
bool Mission::finished() const { return state_ == State::Completed || state_ == State::Missed; }
IncidentType Mission::type() const { return type_; }
bool Mission::needsHospital(IncidentType type) { return incidentInfo(type).needsHospital; }
Rgba Mission::colour() const {
    return state_ == State::Transport ? HospitalColour : incidentInfo(type_).colour;
}
Point Mission::incidentPosition() const { return marker_.position(); }
Point Mission::hospitalPosition() const { return hospital_; }
Point Mission::target() const { return state_ == State::Transport ? hospital_ : marker_.position(); }
float Mission::elapsed() const { return elapsed_; }
float Mission::timeLimit() const { return legLimit_; }
float Mission::drivingTime() const {
    if (state_ == State::Transport || (state_ == State::Completed && needsHospital(type_)))
        return firstLeg_ + elapsed_;
    return state_ == State::Completed ? firstLeg_ : elapsed_;
}
float Mission::distanceRemaining() const { return distance_; }
float Mission::holdProgress() const { return std::min(1.0f, hold_ / HoldTime); }
float Mission::treatmentTime() const { return incidentInfo(type_).treatment; }
float Mission::treatmentProgress() const {
    return std::min(1.0f, treatment_ / std::max(0.01f, treatmentTime()));
}
int Mission::completed() const { return completed_; }
int Mission::missed() const { return missed_; }
float Mission::lastResponseTime() const { return lastResponse_; }
