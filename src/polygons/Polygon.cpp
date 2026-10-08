#include "Polygon.h"
#include "Transformations.h"

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

void Polygon::translate(float dx, float dy) {
    vertices_ = Transformations::translate(vertices_, dx, dy);
}

void Polygon::rotate(float degrees, Point pivot) {
    vertices_ = Transformations::rotate(vertices_, degrees, pivot);
}

void Polygon::rotate(float degrees) {
    rotate(degrees, centroid());
}

void Polygon::scale(float sx, float sy, Point pivot) {
    vertices_ = Transformations::scale(vertices_, sx, sy, pivot);
}

void Polygon::scale(float sx, float sy) {
    scale(sx, sy, centroid());
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
    sf::CircleShape marker(4.0f);
    marker.setOrigin(4.0f, 4.0f);
    marker.setFillColor(color);

    for (const Point& p : vertices_) {
        marker.setPosition(p.x, p.y);
        target.draw(marker);
    }
}
