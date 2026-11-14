#include "Scoring.h"
#include <algorithm>
#include <cmath>

namespace Scoring {

int baseScore(int incidentType) {
    switch (incidentType) {
        case 0: return 500;   // medical: most urgent
        case 1: return 400;   // fire
        case 2: return 450;   // road accident
    }
    return 400;
}

float parTime(float routeLength) {
    return std::max(0.0f, routeLength) / ParSpeed + StopAllowance;
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
    s.parTime = parTime(r.routeLength);
    if (!r.arrived) return s;                          // missed call: nothing

    s.base = baseScore(r.incidentType);
    const float saved = std::clamp(s.parTime - r.elapsed, 0.0f, MaxBonusSeconds);
    s.timeBonus = static_cast<int>(std::lround(saved * BonusPerSecond));
    s.collisionPenalty = std::max(0, r.collisions) * CollisionPenalty;
    s.redLightPenalty = std::max(0, r.redLights) * RedLightPenalty;
    s.total = std::max(0, s.base + s.timeBonus - s.collisionPenalty - s.redLightPenalty);
    s.grade = grade(s.total, s.base);
    return s;
}

}
