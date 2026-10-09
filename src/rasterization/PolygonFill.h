#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "../core/Point.h"
#include "../lines/LineRenderer.h"

namespace PolygonFill {
    // Returns horizontal scan-line spans as start/end point pairs.
    std::vector<std::pair<Point, Point>>
    scanLineSpans(const std::vector<Point>& vertices);

    void draw(sf::RenderTarget& target,
              const std::vector<Point>& vertices,
              const LineRenderer& renderer,
              sf::Color color,
              bool debug = false);
}
