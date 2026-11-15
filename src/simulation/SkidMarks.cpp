#include "SkidMarks.h"
#include <algorithm>
#include <cmath>

void SkidMarks::reset() {
    marks_.clear();
    havePrevious_ = false;
}

void SkidMarks::update(float deltaTime, const Ambulance& ambulance) {
    for (Mark& m : marks_) m.age += deltaTime;
    while (!marks_.empty() && marks_.front().age >= Lifetime) marks_.pop_front();

    const float skid = ambulance.skid();
    Point wheels[2];
    ambulance.rearWheels(wheels[0], wheels[1]);

    if (skid < Threshold) {
        havePrevious_ = false;
        return;
    }
    if (havePrevious_) {
        for (int i = 0; i < 2; ++i) {
            // Skip zero-length segments (sliding to a stop).
            if (std::hypot(wheels[i].x - previous_[i].x, wheels[i].y - previous_[i].y) < 0.5f)
                continue;
            marks_.push_back({previous_[i], wheels[i], std::min(1.0f, skid), 0.0f});
        }
        while (marks_.size() > MaxMarks) marks_.pop_front();
    }
    previous_[0] = wheels[0];
    previous_[1] = wheels[1];
    havePrevious_ = true;
}

void SkidMarks::draw(IRenderer& target, const LineRenderer& lines) const {
    for (const Mark& m : marks_) {
        const float fade = 1.0f - m.age / Lifetime;
        const int alpha = static_cast<int>(std::lround(MaxAlpha * m.strength * fade));
        if (alpha <= 2) continue;
        lines.draw(target, m.a, m.b, Rgba(18, 18, 20, alpha), 3.0f);
    }
}
