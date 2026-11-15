#pragma once
#include <algorithm>
#include "../core/Rgba.h"

// Every kind of call in one table, so the mission, HUD, banner, scoring
// and screens agree.
enum class IncidentType { Medical, Fire, Accident, Cardiac, Maternity };
constexpr int IncidentTypeCount = 5;

struct IncidentInfo {
    const char* callName;   // banner and result screen
    const char* hudName;    // seven-segment HUD (letters it can show)
    Rgba colour;            // marker, route and HUD colour
    float timeLimit;        // seconds to reach the scene
    float treatment;        // seconds on scene
    int baseScore;
    bool needsHospital;     // patient taken to hospital afterwards
    float weight;           // share of calls (weights sum to 1)
};

// Calls are picked with these weights: a uniform u in [0, 1) is compared
// with the running sum of the weights (inverse transform sampling).
inline const IncidentInfo& incidentInfo(int type) {
    static const IncidentInfo table[IncidentTypeCount] = {
        {"MEDICAL CALL",   "AID",   Rgba(235, 70, 70),   60.0f, 3.0f, 500, true,  0.30f},
        {"FIRE CALL",      "FIRE",  Rgba(245, 150, 40),  50.0f, 2.0f, 400, false, 0.20f},
        {"ROAD CRASH",     "CRASH", Rgba(240, 210, 60),  70.0f, 4.0f, 450, true,  0.25f},
        {"CARDIAC ARREST", "HEART", Rgba(255, 90, 170),  45.0f, 5.0f, 600, true,  0.15f},
        {"MATERNITY",      "BABY",  Rgba(170, 120, 255), 75.0f, 3.0f, 450, true,  0.10f},
    };
    return table[std::clamp(type, 0, IncidentTypeCount - 1)];
}

inline const IncidentInfo& incidentInfo(IncidentType type) {
    return incidentInfo(static_cast<int>(type));
}

inline IncidentType pickIncident(float u) {
    float sum = 0.0f;
    for (int i = 0; i < IncidentTypeCount; ++i) {
        sum += incidentInfo(i).weight;
        if (u < sum) return static_cast<IncidentType>(i);
    }
    return static_cast<IncidentType>(IncidentTypeCount - 1);
}
