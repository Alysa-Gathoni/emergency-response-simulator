#include "Environment.h"
#include "../shapes/CircleEllipse.h"

void Environment::draw(sf::RenderTarget& target, bool debug) const {
    const Point treePositions[] = {
        {285,110}, {510,115}, {285,520}, {510,540}, {835,520}
    };

    for (Point p : treePositions) {
        // Canopy: filled midpoint ellipse with a darker outline.
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::filledEllipse(p, 18, 27),
            sf::Color(55,130,70),
            1.0f
        );
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::midpointEllipse(p, 18, 27),
            sf::Color(40,100,55),
            2.0f
        );

        // Highlight: filled midpoint circle.
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::filledCircle({p.x - 6, p.y - 8}, 8),
            sf::Color(95,185,105),
            1.0f
        );

        if (debug) {
            ShapeAlgorithms::drawPoints(
                target,
                ShapeAlgorithms::midpointEllipse(p, 23, 32),
                sf::Color(80,220,170),
                1.0f
            );
        }
    }
}
