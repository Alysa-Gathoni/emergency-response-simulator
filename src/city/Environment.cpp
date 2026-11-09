#include "Environment.h"
#include "../shapes/CircleEllipse.h"

void Environment::draw(sf::RenderTarget& target, bool debug) const {
    // Trees stand in the two parks of the district.
    const Point treePositions[] = {
        {60,642}, {124,670}, {190,646}, {1014,50}, {1058,84}
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
