#pragma once
#include "render/IRenderer.h"
#include <deque>
#include "Ambulance.h"
#include "../lines/LineRenderer.h"

// Tyre marks left by the rear wheels while the ambulance slides.
//
// Every frame with Ambulance::skid() above Threshold adds one segment per
// rear wheel, from where the wheel was last frame to where it is now (the
// wheel positions come from the scene graph: world = T(p) * R(h) * mount).
// A mark fades linearly over Lifetime seconds:
//     alpha = MaxAlpha * strength * (1 - age / Lifetime)
// and the oldest marks are dropped beyond MaxMarks, so memory stays fixed.
class SkidMarks {
public:
    static constexpr float Threshold = 0.15f;
    static constexpr float Lifetime = 12.0f;     // seconds
    static constexpr std::size_t MaxMarks = 900;
    static constexpr int MaxAlpha = 170;

    struct Mark {
        Point a;
        Point b;
        float strength;   // skid intensity 0..1 when it was laid
        float age;        // seconds
    };

    void update(float deltaTime, const Ambulance& ambulance);
    void draw(IRenderer& target, const LineRenderer& lines) const;
    void reset();

    const std::deque<Mark>& marks() const { return marks_; }

private:
    std::deque<Mark> marks_;
    bool havePrevious_{false};
    Point previous_[2]{};
};
