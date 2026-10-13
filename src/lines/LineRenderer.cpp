#include "LineRenderer.h"
#include "../core/Raster.h"

LineRenderer::LineRenderer(LineMode mode) : mode_(mode) {}

void LineRenderer::setMode(LineMode mode) { mode_ = mode; }
LineMode LineRenderer::getMode() const { return mode_; }
void LineRenderer::setDebug(bool enabled) { debug_ = enabled; }
bool LineRenderer::isDebug() const { return debug_; }

std::vector<Point> LineRenderer::rasterize(Point start, Point end) const {
    return mode_ == LineMode::Bresenham
        ? LineAlgorithms::bresenham(start, end)
        : LineAlgorithms::dda(start, end);
}

void LineRenderer::draw(sf::RenderTarget& target, Point start, Point end,
                        sf::Color color, float pixelSize) const {
    const auto points = rasterize(start, end);
    Raster::plot(target, points, color, pixelSize);

    if (debug_) {
        for (std::size_t i = 0; i < points.size(); i += 12)
            Raster::marker(target, points[i], 3.0f, sf::Color(80, 220, 170));
    }
}
