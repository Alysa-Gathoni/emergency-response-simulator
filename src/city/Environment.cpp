#include "Environment.h"
#include "../shapes/CircleEllipse.h"

void Environment::draw(sf::RenderTarget& target, bool debug) const {
    const Point treePositions[] = {
        {285,110}, {510,115}, {285,520}, {510,540}, {835,520}
    };

    for (Point p : treePositions) {
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::midpointEllipse(p, 18, 27),
            sf::Color(80,165,95),
            3.0f
        );

        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::midpointCircle({p.x - 8, p.y - 6}, 10),
            sf::Color(95,185,105),
            2.0f
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
