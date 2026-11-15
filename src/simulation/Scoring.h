#pragma once

// Mission scoring.
//
//   score     = max(0, base + timeBonus - collisions * CollisionPenalty
//                                       - redLights  * RedLightPenalty
//                                       - wrongTurns * WrongTurnPenalty)
//   timeBonus = clamp(parTime - elapsed, 0, MaxBonusSeconds) * BonusPerSecond
//   parTime   = routeLength / ParSpeed + StopAllowance * stops
//
// A call with a patient has two legs (scene, then hospital): routeLength
// is the sum of both shortest routes and there are two stops. Time spent
// treating on scene is not counted.
//
// The total is then multiplied by the difficulty (easy 0.8, normal 1,
// hard 1.25); the grade compares the score before that multiplier.
//
// parTime is what a careful driver needs for the shortest route at the
// start of the call, so beating it is rewarded and a reckless fast run
// still loses points for every crash and red light.
struct MissionReport {
    int incidentType{0};       // IncidentType (see Incidents.h)
    bool arrived{false};       // false = time limit exceeded
    float elapsed{0.0f};       // seconds
    float routeLength{0.0f};   // pixels, shortest route(s) when each leg started
    int stops{1};              // 2 when the patient was taken to hospital
    bool delivered{false};     // patient handed over at the hospital
    float multiplier{1.0f};    // difficulty
    int collisions{0};         // during this call only
    int redLights{0};          // during this call only
    int wrongTurns{0};         // times the route had to be recalculated
};

struct ScoreBreakdown {
    int base{0};
    float parTime{0.0f};
    int timeBonus{0};
    int collisionPenalty{0};
    int redLightPenalty{0};
    int wrongTurnPenalty{0};
    int difficultyBonus{0};    // total after the multiplier minus before
    int total{0};
    char grade{'F'};
};

namespace Scoring {

constexpr float ParSpeed = 90.0f;          // px/s, average incl. junctions
constexpr float StopAllowance = 5.0f;      // s, slowing down and holding still
constexpr float MaxBonusSeconds = 30.0f;
constexpr int BonusPerSecond = 10;
constexpr int CollisionPenalty = 50;
constexpr int RedLightPenalty = 30;
constexpr int WrongTurnPenalty = 20;

int baseScore(int incidentType);
float parTime(float routeLength, int stops = 1);
ScoreBreakdown score(const MissionReport& report);

// Grade against the call's base score:
//   A: base + 100 or more (beat par, clean)   B: at least the base
//   C: at least 70% of the base               D: anything above 0   F: 0
char grade(int total, int base);

}
