#pragma once
#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>
#include "Point.h"
#include "render/IRenderer.h"

// Hand-off point between our algorithms and the active renderer.
// Every module produces pixels or scan-line spans itself and submits them
// here in one batch per call; the renderer (software or OpenGL) only puts
// them on screen.
namespace Raster {

// Point clipping: a pixel square is kept when it overlaps the clip
// window (the simplest clipping test: four comparisons). Pixels drawn in
// local coordinates through the hardware transform stack are left to the
// graphics API, as their world position is not known here.
inline void plot(IRenderer& target, const std::vector<Point>& pixels,
                 Rgba color, float size = 1.0f) {
    if (pixels.empty()) return;
    const ClipRect* clip = target.pixelClip();
    if (!clip || target.hasModel()) {
        target.drawPixels(pixels, color, size);
        return;
    }
    std::vector<Point> inside;
    inside.reserve(pixels.size());
    for (const Point& p : pixels)
        if (p.x + size > clip->left && p.x < clip->right && p.y + size > clip->top && p.y < clip->bottom)
            inside.push_back(p);
    if (!inside.empty()) target.drawPixels(inside, color, size);
}

// Horizontal spans (x1..x2 inclusive on row y) from our fill algorithms.
// Span clipping: rows outside the window are dropped, the others cut
// to its left and right edges.
inline void spans(IRenderer& target,
                  const std::vector<std::pair<Point, Point>>& rows,
                  Rgba color) {
    if (rows.empty()) return;
    const ClipRect* clip = target.pixelClip();
    if (!clip || target.hasModel()) {
        target.drawSpans(rows, color);
        return;
    }
    std::vector<std::pair<Point, Point>> inside;
    inside.reserve(rows.size());
    for (const auto& [a, b] : rows) {
        if (a.y + 1.0f <= clip->top || a.y >= clip->bottom) continue;
        const float x1 = std::max(a.x, std::ceil(clip->left));
        const float x2 = std::min(b.x, std::floor(clip->right) - 1.0f);
        if (x2 >= x1) inside.push_back({{x1, a.y}, {x2, b.y}});
    }
    if (!inside.empty()) target.drawSpans(inside, color);
}

// Small square marker used by the debug views.
inline void marker(IRenderer& target, Point centre, float halfSize, Rgba color) {
    plot(target, {Point{centre.x - halfSize, centre.y - halfSize}}, color, halfSize * 2.0f);
}

}
