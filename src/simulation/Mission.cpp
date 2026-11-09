#include "Mission.h"
#include "../shapes/CircleEllipse.h"
#include "../core/Color.h"
#include <algorithm>
#include <cmath>

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float MinSpawnDistance = 260.0f;   // never right next to the ambulance
constexpr float ResultPause = 2.5f;

struct IncidentInfo {
    sf::Color colour;
    float timeLimit;   // seconds
};

IncidentInfo infoFor(IncidentType type) {
    switch (type) {
        case IncidentType::Medical:  return {sf::Color(235, 70, 70), 60.0f};
        case IncidentType::Fire:     return {sf::Color(245, 150, 40), 50.0f};
        case IncidentType::Accident: return {sf::Color(240, 210, 60), 70.0f};
    }
    return {sf::Color::White, 60.0f};
}

}

Mission::Mission(unsigned seed)
    : rng_(seed), marker_({0.0f, 0.0f}) {
    // Incidents happen on the roads between junctions, in either lane,
    // so the ambulance can always stop right at the scene.
    spawnPoints_ = {
        {120, 175}, {560, 225}, {980, 175}, {120, 545},
        {560, 495}, {980, 545}, {275, 360}, {325, 80},
        {775, 640}, {825, 360}, {275, 650}, {825, 70},
    };
    nextIncident({420.0f, 175.0f});
}

void Mission::nextIncident(Point ambulancePosition) {
    std::uniform_int_distribution<int> pick(0, static_cast<int>(spawnPoints_.size()) - 1);
    std::uniform_int_distribution<int> kind(0, 2);

    // Different from the last one and not too close to the ambulance.
    int candidate = pick(rng_);
    for (int tries = 0; tries < 50; ++tries) {
        const Point p = spawnPoints_[candidate];
        const float d = std::hypot(p.x - ambulancePosition.x, p.y - ambulancePosition.y);
        if (candidate != currentSpawn_ && d >= MinSpawnDistance) break;
        candidate = pick(rng_);
    }

    currentSpawn_ = candidate;
    type_ = static_cast<IncidentType>(kind(rng_));
    marker_.setPosition(spawnPoints_[candidate]);
    marker_.setColour(infoFor(type_).colour);
    state_ = State::Active;
    elapsed_ = 0.0f;
    hold_ = 0.0f;
}

void Mission::restart(Point ambulancePosition) {
    rng_.seed(3);
    currentSpawn_ = -1;
    completed_ = 0;
    missed_ = 0;
    lastResponse_ = 0.0f;
    nextIncident(ambulancePosition);
}

void Mission::update(float deltaTime, Point ambulancePosition, float ambulanceSpeed) {
    marker_.update(deltaTime);
    const Point target = marker_.position();
    distance_ = std::hypot(target.x - ambulancePosition.x, target.y - ambulancePosition.y);

    if (state_ != State::Active) {
        pause_ -= deltaTime;
        if (pause_ <= 0.0f) nextIncident(ambulancePosition);
        return;
    }

    elapsed_ += deltaTime;

    const bool atScene = distance_ < ArrivalRadius && std::abs(ambulanceSpeed) < StopSpeed;
    hold_ = atScene ? hold_ + deltaTime : 0.0f;

    if (hold_ >= HoldTime) {
        state_ = State::Arrived;
        ++completed_;
        lastResponse_ = elapsed_;
        pause_ = ResultPause;
    } else if (elapsed_ > timeLimit()) {
        state_ = State::Missed;
        ++missed_;
        pause_ = ResultPause;
    }
}

void Mission::draw(sf::RenderTarget& target, const LineRenderer&, bool debug) const {
    if (state_ == State::Active) marker_.draw(target, debug);

    const Point c = marker_.position();

    if (state_ == State::Active && hold_ > 0.0f) {
        // Hold-still indicator: keep only the midpoint-circle pixels whose
        // angle (measured clockwise from 12 o'clock) is within progress * 2pi.
        const float limit = holdProgress() * 2.0f * PI;
        std::vector<Point> arc;
        for (const Point& p : ShapeAlgorithms::midpointCircle(c, 38)) {
            float a = std::atan2(p.x - c.x, -(p.y - c.y));
            if (a < 0.0f) a += 2.0f * PI;
            if (a <= limit) arc.push_back(p);
        }
        ShapeAlgorithms::drawPoints(target, arc, sf::Color(90, 230, 120), 3.0f);
    }

    if (state_ == State::Arrived) {
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::midpointCircle(c, 30),
                                    sf::Color(90, 230, 120), 4.0f);
    }

    if (debug && state_ == State::Active) {
        ShapeAlgorithms::drawPoints(
            target, ShapeAlgorithms::midpointCircle(c, static_cast<int>(ArrivalRadius)),
            Colors::Debug, 1.0f);
    }
}

Mission::State Mission::state() const { return state_; }
IncidentType Mission::type() const { return type_; }
Point Mission::incidentPosition() const { return marker_.position(); }
float Mission::elapsed() const { return elapsed_; }
float Mission::timeLimit() const { return infoFor(type_).timeLimit; }
float Mission::distanceRemaining() const { return distance_; }
float Mission::holdProgress() const { return std::min(1.0f, hold_ / HoldTime); }
int Mission::completed() const { return completed_; }
int Mission::missed() const { return missed_; }
float Mission::lastResponseTime() const { return lastResponse_; }
