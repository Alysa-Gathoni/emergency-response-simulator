#include "Check.h"
#include "simulation/Mission.h"
#include <cmath>
#include <vector>

namespace {

constexpr float Dt = 1.0f / 60.0f;
const Point Hospital{100, 100};
const std::vector<Point> Spots = {{700, 100}, {100, 700}, {700, 700}, {600, 400}};

// Runs the mission for `seconds` with the ambulance parked at `at`.
void park(Mission& m, Point at, float seconds, float speed = 0.0f) {
    for (int i = 0; i < static_cast<int>(seconds / Dt); ++i) m.update(Dt, at, speed);
}

// First call of a mission with the given seed that has this type.
Mission withType(IncidentType wanted) {
    for (unsigned seed = 1; seed < 200; ++seed) {
        Mission m(Spots, Hospital, seed);
        if (m.type() == wanted) return m;
    }
    return Mission(Spots, Hospital, 1);
}

void patientGoesToHospital() {
    Mission m = withType(IncidentType::Medical);
    CHECK(m.state() == Mission::State::EnRoute);
    CHECK(m.driving());
    const Point scene = m.incidentPosition();
    CHECK(m.target().x == scene.x && m.target().y == scene.y);

    // Driving through the scene does not count.
    park(m, scene, 2.0f, 60.0f);
    CHECK(m.state() == Mission::State::EnRoute);

    // Stopping there: hold, then treatment.
    park(m, scene, Mission::HoldTime + 0.1f);
    CHECK(m.state() == Mission::State::OnScene);
    CHECK(!m.driving());
    const float legOne = m.elapsed();
    park(m, scene, 1.0f);
    CHECK(m.treatmentProgress() > 0.2f && m.treatmentProgress() < 1.0f);
    CHECK_NEAR(m.elapsed(), legOne, 1e-4f);              // timer paused while treating

    park(m, scene, m.treatmentTime() - 1.0f + 0.05f);
    CHECK(m.state() == Mission::State::Transport);
    CHECK(m.target().x == Hospital.x && m.target().y == Hospital.y);
    CHECK(m.elapsed() < 0.1f);                           // new leg, new timer
    const float manhattan = std::abs(Hospital.x - scene.x) + std::abs(Hospital.y - scene.y);
    CHECK_NEAR(m.timeLimit(), manhattan / Mission::TransportSpeed + Mission::TransportAllowance, 1e-3f);

    park(m, {Hospital.x + 300, Hospital.y}, 3.0f);       // driving back
    park(m, Hospital, Mission::HoldTime + 0.1f);
    CHECK(m.state() == Mission::State::Completed);
    CHECK(m.finished());
    CHECK(m.completed() == 1);
    CHECK(m.lastResponseTime() > legOne + 3.0f);         // both legs counted
    CHECK(m.lastResponseTime() < legOne + 5.0f);         // treatment excluded

    // After the pause the next call comes in.
    park(m, Hospital, 3.0f);
    CHECK(m.state() == Mission::State::EnRoute);
}

void fireNeedsNoHospital() {
    CHECK(!Mission::needsHospital(IncidentType::Fire));
    CHECK(Mission::needsHospital(IncidentType::Accident));
    Mission m = withType(IncidentType::Fire);
    const Point scene = m.incidentPosition();
    park(m, scene, Mission::HoldTime + m.treatmentTime() + 0.3f);
    CHECK(m.state() == Mission::State::Completed);
}

void leavingTheSceneAbandonsTreatment() {
    Mission m = withType(IncidentType::Accident);
    const Point scene = m.incidentPosition();
    park(m, scene, Mission::HoldTime + 0.5f);
    CHECK(m.state() == Mission::State::OnScene);
    park(m, {scene.x + 200, scene.y}, Dt * 2, 80.0f);
    CHECK(m.state() == Mission::State::EnRoute);
    CHECK(m.treatmentProgress() == 0.0f);
}

void lateMeansMissed() {
    Mission m(Spots, Hospital, 5);
    park(m, Hospital, m.timeLimit() + 0.5f);
    CHECK(m.state() == Mission::State::Missed);
    CHECK(m.missed() == 1);
    CHECK(m.completed() == 0);
}

}

int main() {
    patientGoesToHospital();
    fireNeedsNoHospital();
    leavingTheSceneAbandonsTreatment();
    lateMeansMissed();
    return check::failures();
}
