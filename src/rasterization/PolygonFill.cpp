#include "PolygonFill.h"
#include <algorithm>
#include <cmath>

namespace PolygonFill {

std::vector<std::pair<Point, Point>>
scanLineSpans(const std::vector<Point>& vertices) {
    std::vector<std::pair<Point, Point>> spans;
    if (vertices.size() < 3) return spans;

    float minY = vertices.front().y;
    float maxY = vertices.front().y;
    for (const auto& p : vertices) {
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    const int yStart = static_cast<int>(std::ceil(minY));
    const int yEnd   = static_cast<int>(std::floor(maxY));

    for (int y = yStart; y <= yEnd; ++y) {
        std::vector<float> intersections;

        for (std::size_t i = 0; i < vertices.size(); ++i) {
            const Point& a = vertices[i];
            const Point& b = vertices[(i + 1) % vertices.size()];

            // Half-open edge rule avoids double-counting shared vertices.
            const bool crosses =
                (a.y <= y && b.y > y) ||
                (b.y <= y && a.y > y);

            if (!crosses) continue;

            const float x = a.x +
                (static_cast<float>(y) - a.y) *
                (b.x - a.x) / (b.y - a.y);

            intersections.push_back(x);
        }

        std::sort(intersections.begin(), intersections.end());

        for (std::size_t i = 0; i + 1 < intersections.size(); i += 2) {
            spans.push_back({
                {std::ceil(intersections[i]), static_cast<float>(y)},
                {std::floor(intersections[i + 1]), static_cast<float>(y)}
            });
        }
    }

    return spans;
}

void draw(sf::RenderTarget& target,
          const std::vector<Point>& vertices,
          const LineRenderer& renderer,
          sf::Color color,
          bool debug) {
    const auto spans = scanLineSpans(vertices);

    for (std::size_t i = 0; i < spans.size(); ++i) {
        sf::Color spanColor = color;
        if (debug && i % 8 == 0)
            spanColor = sf::Color(80, 220, 170);

        renderer.draw(target, spans[i].first, spans[i].second,
                      spanColor, 1.0f);
    }
}

}
