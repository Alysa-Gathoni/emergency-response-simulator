#pragma once
#include "render/IRenderer.h"
#include <string>
#include "../lines/LineRenderer.h"

// Values shown on the heads-up display, collected by the main loop from
// the mission, ambulance, collision and red-light systems.
struct HudData {
    int incidentType{0};        // 0 medical, 1 fire, 2 accident
    float timeLeft{0.0f};       // seconds
    float timeLimit{1.0f};
    float distance{0.0f};       // pixels to the incident
    float speed{0.0f};          // pixels per second
    int collisions{0};
    int redLights{0};
    int completed{0};
    int missed{0};
    bool arrived{false};
    bool late{false};
    int lineMode{0};            // 0 Bresenham, 1 DDA, 2 graphics API lines
};

// Scale used for display only: 1 pixel = 0.2 m.
constexpr float MetresPerPixel = 0.2f;

class Hud {
public:
    void draw(IRenderer& target, const LineRenderer& lines,
              const HudData& data) const;

    // mm:ss for a number of seconds (negative shown as 00:00).
    static std::string clock(float seconds);
    static int kilometresPerHour(float pixelsPerSecond);
    static int metres(float pixels);
};
