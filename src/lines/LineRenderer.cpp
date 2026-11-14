#include "LineRenderer.h"
#include "../core/Raster.h"

LineRenderer::LineRenderer(LineMode mode) : mode_(mode) {}

void LineRenderer::setMode(LineMode mode) { mode_ = mode; }

void LineRenderer::nextMode() {
    switch (mode_) {
        case LineMode::Bresenham: mode_ = LineMode::DDA; break;
        case LineMode::DDA:       mode_ = LineMode::Native; break;
        case LineMode::Native:    mode_ = LineMode::Bresenham; break;
    }
}
LineMode LineRenderer::getMode() const { return mode_; }
void LineRenderer::setDebug(bool enabled) { debug_ = enabled; }
bool LineRenderer::isDebug() const { return debug_; }

std::vector<Point> LineRenderer::rasterize(Point start, Point end) const {
    return mode_ == LineMode::DDA ? LineAlgorithms::dda(start, end)
                                  : LineAlgorithms::bresenham(start, end);
}

void LineRenderer::draw(IRenderer& target, Point start, Point end,
                        Rgba color, float pixelSize) const {
    if (mode_ == LineMode::Native) {
        // Centre the API line on the same pixel squares our algorithms fill.
        const float h = pixelSize * 0.5f;
        target.drawNativeLines({{start.x + h, start.y + h}, {end.x + h, end.y + h}},
                               color, pixelSize);
        return;
    }

    const auto points = rasterize(start, end);
    Raster::plot(target, points, color, pixelSize);

    if (debug_) {
        for (std::size_t i = 0; i < points.size(); i += 12)
            Raster::marker(target, points[i], 3.0f, Rgba(80, 220, 170));
    }
}
