#include "DayNight.h"
#include "RoadNetwork.h"
#include "../polygons/Polygon.h"
#include "../rasterization/Clipping.h"
#include "../rasterization/PolygonFill.h"
#include "../shapes/CircleEllipse.h"
#include "../core/Color.h"
#include <algorithm>
#include <cmath>

namespace {

constexpr float PI = 3.14159265358979323846f;

struct Keyframe {
    float t;             // time of day 0..1
    float r, g, b, a;    // overlay colour and opacity (0..255)
};

// Day -> orange dusk -> deep blue night -> pink dawn -> day.
const Keyframe Keys[] = {
    {0.00f,   0,   0,   0,   0},
    {0.30f,   0,   0,   0,   0},
    {0.40f, 255, 120,  40,  70},
    {0.50f,  10,  20,  60, 165},
    {0.80f,  10,  20,  60, 165},
    {0.90f, 255, 160, 110,  60},
    {1.00f,   0,   0,   0,   0},
};

float lerp(float a, float b, float u) { return a + (b - a) * u; }

}

void DayNight::update(float deltaTime) {
    if (mode_ == Mode::Cycle)
        time_ = std::fmod(time_ + deltaTime / DayLength, 1.0f);
}

void DayNight::nextMode() {
    switch (mode_) {
        case Mode::Day:   mode_ = Mode::Cycle; time_ = 0.25f; break;
        case Mode::Cycle: mode_ = Mode::Night; time_ = 0.6f; break;
        case Mode::Night: mode_ = Mode::Day;   time_ = 0.0f; break;
    }
}

DayNight::Mode DayNight::mode() const { return mode_; }
float DayNight::timeOfDay() const { return time_; }

Rgba DayNight::tint() const {
    // Find the two keyframes around the current time and interpolate.
    for (std::size_t i = 0; i + 1 < std::size(Keys); ++i) {
        const Keyframe& k0 = Keys[i];
        const Keyframe& k1 = Keys[i + 1];
        if (time_ < k0.t || time_ > k1.t) continue;
        const float u = (time_ - k0.t) / (k1.t - k0.t);
        return Rgba(static_cast<std::uint8_t>(lerp(k0.r, k1.r, u)),
                         static_cast<std::uint8_t>(lerp(k0.g, k1.g, u)),
                         static_cast<std::uint8_t>(lerp(k0.b, k1.b, u)),
                         static_cast<std::uint8_t>(lerp(k0.a, k1.a, u)));
    }
    return Rgba::Transparent;
}

float DayNight::darkness() const {
    return std::clamp(tint().a / 165.0f, 0.0f, 1.0f);
}

void DayNight::drawOverlay(IRenderer& target, const LineRenderer& lines,
                           float w, float h) const {
    const Rgba c = tint();
    if (c.a == 0) return;
    PolygonFill::draw(target, {{0, 0}, {w, 0}, {w, h}, {0, h}}, lines, c, false);
}

void DayNight::drawLights(IRenderer& target, const LineRenderer& lines,
                          const std::vector<Light>& lights, const ClipRect& visible,
                          bool debug) const {
    const float dark = darkness();
    if (dark < 0.15f) return;

    const ClipRect screen = visible;
    // Two nested cones (wide and dim, narrow and brighter) give a soft
    // falloff without per-pixel lighting.
    const struct { float spreadDeg, reach; } layers[] = {{24.0f, 95.0f}, {11.0f, 80.0f}};
    for (const Light& light : lights) {
        Rgba c = light.colour;

        if (light.direction.x == 0.0f && light.direction.y == 0.0f) {
            // Round glow (signal lamps): a faint filled midpoint circle.
            c.a = static_cast<std::uint8_t>(90.0f * dark);
            ShapeAlgorithms::drawPoints(
                target, ShapeAlgorithms::filledCircle(light.position, 9), c, 1.0f);
            continue;
        }

        // Headlight cone: a triangle from the lamp, rotated +-spread about
        // the direction of travel ...
        const float a = std::atan2(light.direction.y, light.direction.x);
        const Point p = light.position;
        for (const auto& layer : layers) {
            const float spread = layer.spreadDeg * PI / 180.0f;
            const std::vector<Point> cone = {
                p,
                {p.x + layer.reach * std::cos(a - spread), p.y + layer.reach * std::sin(a - spread)},
                {p.x + layer.reach * std::cos(a + spread), p.y + layer.reach * std::sin(a + spread)},
            };

            // ... clipped to the screen with Sutherland-Hodgman, so cones of
            // vehicles near the edge stay valid polygons for the scan-line fill.
            const auto clipped = Clipping::sutherlandHodgman(cone, screen);
            if (clipped.size() < 3) continue;

            c.a = static_cast<std::uint8_t>(38.0f * dark);
            PolygonFill::draw(target, clipped, lines, c, false);
            if (debug)
                Polygon(clipped).drawOutline(target, lines, Colors::Debug, 1.0f);
        }
    }
}
