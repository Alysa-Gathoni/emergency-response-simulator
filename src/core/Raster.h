#pragma once
#include <utility>
#include <vector>
#include "Point.h"
#include "render/IRenderer.h"

// Hand-off point between our algorithms and the active renderer.
// Every module produces pixels or scan-line spans itself and submits them
// here in one batch per call; the renderer (software or OpenGL) only puts
// them on screen.
namespace Raster {

inline void plot(IRenderer& target, const std::vector<Point>& pixels,
                 Rgba color, float size = 1.0f) {
    if (!pixels.empty()) target.drawPixels(pixels, color, size);
}

// Horizontal spans (x1..x2 inclusive on row y) from our fill algorithms.
inline void spans(IRenderer& target,
                  const std::vector<std::pair<Point, Point>>& rows,
                  Rgba color) {
    if (!rows.empty()) target.drawSpans(rows, color);
}

// Small square marker used by the debug views.
inline void marker(IRenderer& target, Point centre, float halfSize, Rgba color) {
    plot(target, {Point{centre.x - halfSize, centre.y - halfSize}}, color, halfSize * 2.0f);
}

}
