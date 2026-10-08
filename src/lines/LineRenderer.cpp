#include "LineRenderer.h"

LineRenderer::LineRenderer(LineMode mode) : mode_(mode) {}

void LineRenderer::setMode(LineMode mode) { mode_ = mode; }
LineMode LineRenderer::getMode() const { return mode_; }
void LineRenderer::setDebug(bool enabled) { debug_ = enabled; }
bool LineRenderer::isDebug() const { return debug_; }

void LineRenderer::draw(sf::RenderTarget& target, Point start, Point end,
                        sf::Color color, float pixelSize) const {
    const auto points = mode_ == LineMode::Bresenham
        ? LineAlgorithms::bresenham(start, end)
        : LineAlgorithms::dda(start, end);

    sf::RectangleShape pixel({pixelSize, pixelSize});
    pixel.setFillColor(color);

    for (const Point& p : points) {
        pixel.setPosition(p.x, p.y);
        target.draw(pixel);
    }

    if (debug_) {
        sf::CircleShape marker(3.0f);
        marker.setOrigin(3.0f, 3.0f);
        marker.setFillColor(sf::Color(80, 220, 170));
        for (std::size_t i = 0; i < points.size(); i += 12) {
            marker.setPosition(points[i].x, points[i].y);
            target.draw(marker);
        }
    }
}
