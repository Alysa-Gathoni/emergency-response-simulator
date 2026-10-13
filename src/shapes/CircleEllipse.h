#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "../core/Point.h"

namespace ShapeAlgorithms {
    std::vector<Point> midpointCircle(Point center, int radius);
    std::vector<Point> midpointEllipse(Point center, int radiusX, int radiusY);

    // Solid shapes: the midpoint outline is generated first, then each
    // scan row is filled between its leftmost and rightmost outline pixel.
    std::vector<Point> filledCircle(Point center, int radius);
    std::vector<Point> filledEllipse(Point center, int radiusX, int radiusY);

    void drawPoints(sf::RenderTarget& target,
                    const std::vector<Point>& points,
                    sf::Color color,
                    float pixelSize = 2.0f);
}
