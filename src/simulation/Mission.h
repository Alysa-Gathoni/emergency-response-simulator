#pragma once
#include "render/IRenderer.h"
#include <random>
#include <vector>
#include "EmergencyMarker.h"
#include "Incidents.h"
#include "../lines/LineRenderer.h"

// One emergency call after another, as a state machine:
//
//          call comes in
//               |
//               v
//   +------> EnRoute --(time limit of the leg exceeded)-----------> Missed
//   |           |  stopped inside the arrival radius
//   |           v
//   |        OnScene  (treatment: progress only while stopped there;
//   |           |      driving off resets it and goes back to EnRoute)
//   |           |  treatment done
//   |           +---- fire: nothing to carry ----------------> Completed
//   |           v
//   |       Transport --(time limit of the leg exceeded)--------> Missed
//   |           |  stopped at the hospital entrance for HoldTime
//   |           v
//   |       Completed
//   |           |
//   +-- next call after a short pause (Completed and Missed alike)
//
// Stopping means distance < radius AND |speed| < StopSpeed (ArrivalRadius
// at a scene, the wider HospitalRadius at the hospital bay): the
// ambulance has to actually stop, not drive through. Each leg has its own
// timer; the transport limit comes from the Manhattan distance to the
// hospital (roads form a grid, so it is the shortest possible road
// distance): limit = (|dx| + |dy|) / TransportSpeed + TransportAllowance.
class Mission {
public:
    enum class State { EnRoute, OnScene, Transport, Completed, Missed };

    static constexpr float ArrivalRadius = 45.0f;
    static constexpr float HospitalRadius = 90.0f;     // the bay covers both lanes
    static constexpr float StopSpeed = 15.0f;
    static constexpr float HoldTime = 1.0f;            // to hand over at the hospital
    static constexpr float TransportSpeed = 45.0f;     // px/s for the transport limit
    static constexpr float TransportAllowance = 25.0f; // s, includes turning round

    // `spawnPoints` are lane positions where incidents can happen (from the
    // road graph); `hospital` is the ambulance bay, where the shift starts
    // and patients are delivered.
    Mission(std::vector<Point> spawnPoints, Point hospital, unsigned seed = 3);

    void update(float deltaTime, Point ambulancePosition, float ambulanceSpeed);
    void draw(IRenderer& target, const LineRenderer& lines, bool debug) const;

    // Starts again from the first call (R key).
    void restart(Point ambulancePosition);

    State state() const;
    bool driving() const;             // EnRoute or Transport: a route is shown
    bool finished() const;            // Completed or Missed (pause before the next call)
    IncidentType type() const;
    static bool needsHospital(IncidentType type);
    Rgba colour() const;              // marker colour of the current incident
    Point incidentPosition() const;
    Point hospitalPosition() const;
    Point target() const;             // where to drive now (scene or hospital)

    float elapsed() const;            // seconds in the current leg
    float timeLimit() const;          // of the current leg
    float drivingTime() const;        // both legs so far (treatment excluded)
    float distanceRemaining() const;  // straight line to target(), pixels
    float holdProgress() const;       // 0..1 while stopping at the target
    float treatmentTime() const;      // seconds needed on scene for this call
    float treatmentProgress() const;  // 0..1
    int completed() const;
    int missed() const;
    float lastResponseTime() const;   // driving time of the last completed call

private:
    std::vector<Point> spawnPoints_;
    Point hospital_;
    std::mt19937 rng_;
    EmergencyMarker marker_;
    EmergencyMarker hospitalMarker_;

    State state_{State::EnRoute};
    IncidentType type_{IncidentType::Medical};
    int currentSpawn_{-1};
    float elapsed_{0.0f};
    float firstLeg_{0.0f};
    float legLimit_{60.0f};
    float hold_{0.0f};
    float treatment_{0.0f};
    float pause_{0.0f};
    float distance_{0.0f};
    int completed_{0};
    int missed_{0};
    float lastResponse_{0.0f};

    void nextIncident(Point ambulancePosition);
    void complete();
    void miss();
};
