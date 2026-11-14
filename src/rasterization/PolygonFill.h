#pragma once
#include "render/IRenderer.h"
#include <vector>
#include "../core/Point.h"
#include "../lines/LineRenderer.h"

namespace PolygonFill {
    // Returns horizontal scan-line spans as start/end point pairs.
    std::vector<std::pair<Point, Point>>
    scanLineSpans(const std::vector<Point>& vertices);

    void draw(IRenderer& target,
              const std::vector<Point>& vertices,
              const LineRenderer& renderer,
              Rgba color,
              bool debug = false);
}
