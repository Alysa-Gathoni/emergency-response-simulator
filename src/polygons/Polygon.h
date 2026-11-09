#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "../core/Mat3.h"
#include "../core/Point.h"
#include "../lines/LineRenderer.h"

class Polygon {
public:
    Polygon() = default;
    explicit Polygon(std::vector<Point> vertices);

    const std::vector<Point>& vertices() const;

    Point centroid() const;

    void translate(float dx, float dy);
    void rotate(float degrees, Point pivot);
    void rotate(float degrees);
    void scale(float sx, float sy, Point pivot);
    void scale(float sx, float sy);

    // One composite matrix (e.g. T * R * S) applied to all vertices at once.
    void transform(const Mat3& matrix);
    Polygon transformed(const Mat3& matrix) const;

    void drawOutline(sf::RenderTarget& target,
                     const LineRenderer& renderer,
                     sf::Color color,
                     float pixelSize = 2.0f) const;

    void drawVertices(sf::RenderTarget& target, sf::Color color) const;

private:
    std::vector<Point> vertices_;
};
