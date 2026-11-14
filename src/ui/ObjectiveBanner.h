#pragma once
#include "render/IRenderer.h"
#include <string>
#include "../lines/LineRenderer.h"

// What the trainee should do right now, in plain words, at the top of
// the screen. The text follows the mission state:
//
//   driving there   "MEDICAL CALL - 350 M TO GO" / "FOLLOW THE ROUTE, ..."
//   close to it     "... SLOW DOWN AND STOP ON THE MARKER"
//   stopping        "... HOLD STILL"
//   arrived/missed  "ARRIVED IN 23 S" or "TOO LATE" / "NEXT CALL COMING UP"
struct BannerData {
    enum class Phase { EnRoute, Arrived, Missed };

    Phase phase{Phase::EnRoute};
    int incidentType{0};        // 0 medical, 1 fire, 2 accident
    int metresToGo{0};          // along the route
    bool nearScene{false};      // close enough to start stopping
    bool holding{false};        // stopped inside the arrival radius
    float responseTime{0.0f};   // seconds, once arrived
};

class ObjectiveBanner {
public:
    static constexpr float Top = 8.0f;

    struct Lines { std::string headline; std::string instruction; };
    static Lines text(const BannerData& data);

    // `leftLimit` keeps the banner clear of the controls panel.
    void draw(IRenderer& target, const LineRenderer& lines,
              const BannerData& data, float leftLimit) const;
};
