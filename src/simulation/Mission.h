#pragma once
#include "render/IRenderer.h"
#include <random>
#include <vector>
#include "EmergencyMarker.h"
#include "../lines/LineRenderer.h"

enum class IncidentType { Medical, Fire, Accident };

// One emergency call after another.
//
//   Active --(inside radius, nearly stopped for HoldTime)--> Arrived
//   Active --(time limit exceeded)-------------------------> Missed
//   Arrived / Missed --(short pause)--> next incident (Active)
//
// Arrival needs the ambulance to actually stop at the scene, not just
// drive through it: distance < ArrivalRadius AND |speed| < StopSpeed,
// held continuously for HoldTime seconds.
class Mission {
public:
    enum class State { Active, Arrived, Missed };

    static constexpr float ArrivalRadius = 45.0f;
    static constexpr float StopSpeed = 15.0f;
    static constexpr float HoldTime = 1.0f;

    // `spawnPoints` are lane positions where incidents can happen (from the
    // road graph); `start` is where the ambulance begins.
    Mission(std::vector<Point> spawnPoints, Point start, unsigned seed = 3);

    void update(float deltaTime, Point ambulancePosition, float ambulanceSpeed);
    void draw(IRenderer& target, const LineRenderer& lines, bool debug) const;

    // Starts again from the first call (R key).
    void restart(Point ambulancePosition);

    State state() const;
    IncidentType type() const;
    Rgba colour() const;         // marker colour of the current incident
    Point incidentPosition() const;
    float elapsed() const;            // seconds since the call came in
    float timeLimit() const;
    float distanceRemaining() const;  // straight line, pixels
    float holdProgress() const;       // 0..1 while stopping at the scene
    int completed() const;
    int missed() const;
    float lastResponseTime() const;

private:
    std::vector<Point> spawnPoints_;
    std::mt19937 rng_;
    EmergencyMarker marker_;

    State state_{State::Active};
    IncidentType type_{IncidentType::Medical};
    int currentSpawn_{-1};
    float elapsed_{0.0f};
    float hold_{0.0f};
    float pause_{0.0f};
    float distance_{0.0f};
    int completed_{0};
    int missed_{0};
    float lastResponse_{0.0f};

    void nextIncident(Point ambulancePosition);
};
