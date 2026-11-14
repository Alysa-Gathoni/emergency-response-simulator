#include "Check.h"
#include "simulation/Scoring.h"

namespace {

void parTimeFollowsRouteLength() {
    CHECK_NEAR(Scoring::parTime(0.0f), Scoring::StopAllowance, 1e-4f);
    CHECK_NEAR(Scoring::parTime(900.0f), 900.0f / Scoring::ParSpeed + Scoring::StopAllowance, 1e-4f);
}

void cleanFastRun() {
    MissionReport r;
    r.incidentType = 0;           // medical, base 500
    r.arrived = true;
    r.routeLength = 900.0f;       // par = 10 + 5 = 15 s
    r.elapsed = 10.0f;            // 5 s under par
    const auto s = Scoring::score(r);
    CHECK(s.base == 500);
    CHECK(s.timeBonus == 50);
    CHECK(s.total == 550);
    CHECK(s.grade == 'B');
}

void bonusIsCapped() {
    MissionReport r;
    r.arrived = true;
    r.routeLength = 9000.0f;      // par 105 s
    r.elapsed = 1.0f;
    const auto s = Scoring::score(r);
    CHECK(s.timeBonus == static_cast<int>(Scoring::MaxBonusSeconds) * Scoring::BonusPerSecond);
    CHECK(s.grade == 'A');
}

void noBonusWhenSlow() {
    MissionReport r;
    r.incidentType = 1;           // fire, base 400
    r.arrived = true;
    r.routeLength = 450.0f;
    r.elapsed = 60.0f;
    const auto s = Scoring::score(r);
    CHECK(s.timeBonus == 0);
    CHECK(s.total == 400);
}

void penaltiesNeverGoNegative() {
    MissionReport r;
    r.incidentType = 2;
    r.arrived = true;
    r.elapsed = 100.0f;
    r.collisions = 20;
    r.redLights = 5;
    const auto s = Scoring::score(r);
    CHECK(s.collisionPenalty == 20 * Scoring::CollisionPenalty);
    CHECK(s.redLightPenalty == 5 * Scoring::RedLightPenalty);
    CHECK(s.total == 0);
    CHECK(s.grade == 'F');
}

void missedCallScoresNothing() {
    MissionReport r;
    r.arrived = false;
    r.elapsed = 70.0f;
    CHECK(Scoring::score(r).total == 0);
}

void grades() {
    CHECK(Scoring::grade(600, 500) == 'A');
    CHECK(Scoring::grade(500, 500) == 'B');
    CHECK(Scoring::grade(350, 500) == 'C');
    CHECK(Scoring::grade(100, 500) == 'D');
    CHECK(Scoring::grade(0, 500) == 'F');
}

}

int main() {
    parTimeFollowsRouteLength();
    cleanFastRun();
    bonusIsCapped();
    noBonusWhenSlow();
    penaltiesNeverGoNegative();
    missedCallScoresNothing();
    grades();
    return check::failures();
}
