#pragma once
#include "render/IRenderer.h"
#include <vector>
#include "../core/Point.h"
#include "../lines/LineRenderer.h"

// How filled polygons reach the screen (F key):
//   ScanLine  our scan-line fill over the whole polygon (default)
//   Triangles ear clipping first, then our scan-line fill per triangle,
//             with the triangle edges drawn so the split is visible
//   Native    ear clipping, then the triangles go to the graphics API
//             (GL_TRIANGLES), which can only fill convex primitives
enum class FillMode { ScanLine, Triangles, Native };

namespace PolygonFill {
    void setMode(FillMode mode);
    void nextMode();
    FillMode mode();
    const char* modeName(FillMode mode);

    // Returns horizontal scan-line spans as start/end point pairs.
    std::vector<std::pair<Point, Point>>
    scanLineSpans(const std::vector<Point>& vertices);

    void draw(IRenderer& target,
              const std::vector<Point>& vertices,
              const LineRenderer& renderer,
              Rgba color,
              bool debug = false);
}
