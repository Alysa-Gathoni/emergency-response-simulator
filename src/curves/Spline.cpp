#include "Spline.h"
#include "Bezier.h"
#include "CatmullRom.h"
#include <algorithm>
#include <cmath>

SplinePath SplinePath::fromCatmullRom(const std::vector<Point>& knots,
                                      bool closed) {
    SplinePath path;
    path.knots_ = knots;
    path.segments_ = CatmullRom::toBezier(knots, closed);
    path.rebuildArcTable();
    return path;
}

void SplinePath::addSegment(const BezierSegment& segment) {
    segments_.push_back(segment);
    rebuildArcTable();
}

// Samples every segment and stores the cumulative chord length, giving a
// lookup table from distance travelled to (segment, t).
void SplinePath::rebuildArcTable() {
    arcTable_.clear();
    if (segments_.empty()) return;

    float total = 0.0f;
    Point previous = segments_.front().p0;
    arcTable_.push_back({0.0f, 0, 0.0f});

    for (std::size_t s = 0; s < segments_.size(); ++s) {
        const auto& seg = segments_[s];
        for (int k = 1; k <= SamplesPerSegment; ++k) {
            const float t = static_cast<float>(k) / SamplesPerSegment;
            const Point p = Bezier::cubic(t, seg.p0, seg.p1, seg.p2, seg.p3);
            total += std::hypot(p.x - previous.x, p.y - previous.y);
            arcTable_.push_back({total, s, t});
            previous = p;
        }
    }
}

Point SplinePath::evaluate(float progress) const {
    if (segments_.empty()) return {};

    progress = progress - std::floor(progress);
    const float target = progress * length();

    // First table entry whose cumulative length reaches the target.
    auto it = std::lower_bound(
        arcTable_.begin() + 1, arcTable_.end(), target,
        [](const ArcSample& sample, float value) {
            return sample.length < value;
        });
    if (it == arcTable_.end()) --it;

    const ArcSample& b = *it;
    const ArcSample& a = *(it - 1);

    // Interpolate t inside b's segment (a may be the end of the previous one).
    const float startT = a.segment == b.segment ? a.t : 0.0f;
    const float span = b.length - a.length;
    const float f = span > 0.0f ? (target - a.length) / span : 0.0f;
    const float t = startT + f * (b.t - startT);

    const auto& s = segments_[b.segment];
    return Bezier::cubic(t, s.p0, s.p1, s.p2, s.p3);
}

Point SplinePath::tangent(float progress) const {
    constexpr float epsilon = 0.002f;
    Point a = evaluate(progress);
    Point b = evaluate(progress + epsilon);
    return {b.x - a.x, b.y - a.y};
}

float SplinePath::length() const {
    return arcTable_.empty() ? 0.0f : arcTable_.back().length;
}

std::vector<Point> SplinePath::sampledPoints(int segmentsPerCurve) const {
    std::vector<Point> result;

    for (const auto& s : segments_) {
        auto sample = Bezier::sampleCubic(
            s.p0, s.p1, s.p2, s.p3, segmentsPerCurve
        );

        if (!result.empty() && !sample.empty())
            sample.erase(sample.begin());

        result.insert(result.end(), sample.begin(), sample.end());
    }

    return result;
}

const std::vector<BezierSegment>& SplinePath::segments() const {
    return segments_;
}

const std::vector<Point>& SplinePath::knots() const {
    return knots_;
}

bool SplinePath::empty() const {
    return segments_.empty();
}
