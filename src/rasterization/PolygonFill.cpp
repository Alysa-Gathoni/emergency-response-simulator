#include "PolygonFill.h"
#include "Clipping.h"
#include "../core/Raster.h"
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

void draw(IRenderer& target,
          const std::vector<Point>& vertices,
          const LineRenderer& renderer,
          Rgba color,
          bool debug) {
    // Camera culling: skip polygons outside the clip window and cut the
    // ones crossing its edge with Sutherland-Hodgman before filling.
    std::vector<Point> clipped;
    const std::vector<Point>* shape = &vertices;
    if (const ClipRect* clip = renderer.clip()) {
        // Shapes drawn in local coordinates through the hardware transform
        // stack are tested in world space and culled whole; OpenGL clips the
        // ones that cross the window edge.
        const bool local = target.hasModel();
        const std::vector<Point> world = local ? target.model().apply(vertices) : std::vector<Point>{};
        const std::vector<Point>& test = local ? world : vertices;

        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        for (const Point& p : test) {
            minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
        }
        if (maxX < clip->left || minX > clip->right || maxY < clip->top || minY > clip->bottom)
            return;
        if (!local &&
            (minX < clip->left || maxX > clip->right || minY < clip->top || maxY > clip->bottom)) {
            clipped = Clipping::sutherlandHodgman(vertices, *clip);
            if (clipped.size() < 3) return;
            shape = &clipped;
        }
    }

    auto spans = scanLineSpans(*shape);

    // Spans whose rounded ends cross (slivers thinner than a pixel) are empty.
    spans.erase(std::remove_if(spans.begin(), spans.end(),
                               [](const auto& s) { return s.second.x < s.first.x; }),
                spans.end());

    if (!debug) {
        Raster::spans(target, spans, color);
        return;
    }

    // Debug view: every 8th span is rasterized pixel by pixel with the
    // selected line algorithm and highlighted, so the scan lines are visible.
    std::vector<std::pair<Point, Point>> plain;
    std::vector<Point> highlighted;
    for (std::size_t i = 0; i < spans.size(); ++i) {
        if (i % 8 == 0) {
            const auto row = renderer.rasterize(spans[i].first, spans[i].second);
            highlighted.insert(highlighted.end(), row.begin(), row.end());
        } else {
            plain.push_back(spans[i]);
        }
    }

    Raster::spans(target, plain, color);
    Raster::plot(target, highlighted, Rgba(80, 220, 170), 1.0f);
}

}
