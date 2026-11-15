#include "Scoring.h"
#include "Incidents.h"
#include <algorithm>
#include <cmath>

namespace Scoring {

int baseScore(int incidentType) {
    return incidentInfo(incidentType).baseScore;
}

float parTime(float routeLength, int stops) {
    return std::max(0.0f, routeLength) / ParSpeed + StopAllowance * std::max(1, stops);
}

char grade(int total, int base) {
    if (total <= 0 || base <= 0) return 'F';
    if (total >= base + 100) return 'A';
    if (total >= base) return 'B';
    if (total * 10 >= base * 7) return 'C';
    return 'D';
}

ScoreBreakdown score(const MissionReport& r) {
    ScoreBreakdown s;
    s.parTime = parTime(r.routeLength, r.stops);
    if (!r.arrived) return s;                          // missed call: nothing

    s.base = baseScore(r.incidentType);
    const float saved = std::clamp(s.parTime - r.elapsed, 0.0f, MaxBonusSeconds);
    s.timeBonus = static_cast<int>(std::lround(saved * BonusPerSecond));
    s.collisionPenalty = std::max(0, r.collisions) * CollisionPenalty;
    s.redLightPenalty = std::max(0, r.redLights) * RedLightPenalty;
    s.wrongTurnPenalty = std::max(0, r.wrongTurns) * WrongTurnPenalty;
    s.total = std::max(0, s.base + s.timeBonus - s.collisionPenalty - s.redLightPenalty
                              - s.wrongTurnPenalty);
    s.grade = grade(s.total, s.base);
    const int scaled = static_cast<int>(std::lround(s.total * std::max(0.0f, r.multiplier)));
    s.difficultyBonus = scaled - s.total;
    s.total = scaled;
    return s;
}

}
