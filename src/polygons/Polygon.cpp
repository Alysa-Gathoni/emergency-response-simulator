#include "Polygon.h"
#include "Transformations.h"
#include "../core/Raster.h"

Polygon::Polygon(std::vector<Point> vertices)
    : vertices_(std::move(vertices)) {}

const std::vector<Point>& Polygon::vertices() const {
    return vertices_;
}

Point Polygon::centroid() const {
    if (vertices_.empty()) return {};

    float x = 0.0f;
    float y = 0.0f;

    for (const Point& p : vertices_) {
        x += p.x;
        y += p.y;
    }

    return {x / vertices_.size(), y / vertices_.size()};
}

// Each operation builds its homogeneous matrix and goes through transform(),
// so all polygon transformations share one code path.
void Polygon::translate(float dx, float dy) {
    transform(Mat3::translation(dx, dy));
}

void Polygon::rotate(float degrees, Point pivot) {
    transform(Mat3::aboutPivot(Mat3::rotationDegrees(degrees), pivot));
}

void Polygon::rotate(float degrees) {
    rotate(degrees, centroid());
}

void Polygon::scale(float sx, float sy, Point pivot) {
    transform(Mat3::aboutPivot(Mat3::scaling(sx, sy), pivot));
}

void Polygon::scale(float sx, float sy) {
    scale(sx, sy, centroid());
}

void Polygon::transform(const Mat3& matrix) {
    vertices_ = Transformations::transform(vertices_, matrix);
}

Polygon Polygon::transformed(const Mat3& matrix) const {
    return Polygon(matrix.apply(vertices_));
}

void Polygon::drawOutline(sf::RenderTarget& target,
                          const LineRenderer& renderer,
                          sf::Color color,
                          float pixelSize) const {
    if (vertices_.size() < 2) return;

    for (std::size_t i = 0; i < vertices_.size(); ++i) {
        const Point& a = vertices_[i];
        const Point& b = vertices_[(i + 1) % vertices_.size()];
        renderer.draw(target, a, b, color, pixelSize);
    }
}

void Polygon::drawVertices(sf::RenderTarget& target, sf::Color color) const {
    for (const Point& p : vertices_)
        Raster::marker(target, p, 4.0f, color);
}
