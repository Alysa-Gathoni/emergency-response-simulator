#pragma once
#include "render/IRenderer.h"
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

    void drawOutline(IRenderer& target,
                     const LineRenderer& renderer,
                     Rgba color,
                     float pixelSize = 2.0f) const;

    void drawVertices(IRenderer& target, Rgba color) const;

private:
    std::vector<Point> vertices_;
};
