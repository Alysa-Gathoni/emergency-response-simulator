#pragma once
#include "render/IRenderer.h"
#include <vector>
#include "../core/Point.h"
#include "../lines/LineRenderer.h"

// Time of day as a tint over the finished scene.
//
// The tint colour and its opacity are keyframed over the day and
// linearly interpolated between keyframes:
//     C(t) = (1 - u) * C0 + u * C1,   u = (t - t0) / (t1 - t0)
// The overlay is then alpha-blended over the scene:
//     out = alpha * tint + (1 - alpha) * scene
// Lights (headlights, signal glow) are drawn AFTER the overlay so they
// stay bright at night.
class DayNight {
public:
    enum class Mode { Day, Cycle, Night };

    void update(float deltaTime);
    void nextMode();                 // T key: Day -> Cycle -> Night -> Day
    Mode mode() const;

    float timeOfDay() const;         // 0..1, 0.5 = midnight
    float darkness() const;          // 0 (day) .. 1 (full night)
    Rgba tint() const;          // overlay colour incl. alpha

    // A light source for the after-overlay pass.
    struct Light {
        Point position;
        Point direction;   // unit vector; zero for a round glow
        Rgba colour;
    };

    void drawOverlay(IRenderer& target, const LineRenderer& lines) const;
    void drawLights(IRenderer& target, const LineRenderer& lines,
                    const std::vector<Light>& lights, bool debug) const;

private:
    Mode mode_{Mode::Day};
    float time_{0.0f};               // 0..1 through the day
    static constexpr float DayLength = 120.0f;   // seconds per full cycle
};
