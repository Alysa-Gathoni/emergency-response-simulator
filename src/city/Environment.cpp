#include "Environment.h"
#include "../shapes/CircleEllipse.h"

Environment::Environment(const std::vector<Rect>& parks) {
    int k = 0;
    for (const Rect& park : parks) {
        for (float y = park.top + 34.0f; y <= park.bottom - 34.0f; y += 64.0f) {
            for (float x = park.left + 26.0f; x <= park.right - 26.0f; x += 64.0f) {
                const float jx = static_cast<float>((k * 37) % 17) - 8.0f;
                const float jy = static_cast<float>((k * 53) % 13) - 6.0f;
                trees_.push_back({x + jx, y + jy});
                ++k;
            }
        }
    }
}

void Environment::draw(IRenderer& target, bool debug) const {
    for (Point p : trees_) {
        // Canopy: filled midpoint ellipse with a darker outline.
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::filledEllipse(p, 18, 27),
            Rgba(55,130,70),
            1.0f
        );
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::midpointEllipse(p, 18, 27),
            Rgba(40,100,55),
            2.0f
        );

        // Highlight: filled midpoint circle.
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::filledCircle({p.x - 6, p.y - 8}, 8),
            Rgba(95,185,105),
            1.0f
        );

        if (debug) {
            ShapeAlgorithms::drawPoints(
                target,
                ShapeAlgorithms::midpointEllipse(p, 23, 32),
                Rgba(80,220,170),
                1.0f
            );
        }
    }
}
