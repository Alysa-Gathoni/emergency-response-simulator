#pragma once
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>
#include "Point.h"

// The only place where computed pixels are handed to SFML.
// All geometry is produced by our own algorithms; SFML just receives one
// batched list of pixel quads per call instead of one draw call per pixel.
namespace Raster {

inline void plot(sf::RenderTarget& target,
                 const std::vector<Point>& pixels,
                 sf::Color color,
                 float size = 1.0f) {
    if (pixels.empty()) return;

    sf::VertexArray quads(sf::Quads, pixels.size() * 4);

    for (std::size_t i = 0; i < pixels.size(); ++i) {
        const float x = pixels[i].x;
        const float y = pixels[i].y;
        sf::Vertex* quad = &quads[i * 4];

        quad[0].position = {x, y};
        quad[1].position = {x + size, y};
        quad[2].position = {x + size, y + size};
        quad[3].position = {x, y + size};

        for (int k = 0; k < 4; ++k)
            quad[k].color = color;
    }

    target.draw(quads);
}

// Horizontal scan-line spans produced by our fill algorithms. Each span
// (x1..x2 on row y, both inclusive) becomes one 1-pixel-high quad, which is
// identical to plotting every pixel of the span but far fewer vertices.
inline void spans(sf::RenderTarget& target,
                  const std::vector<std::pair<Point, Point>>& rows,
                  sf::Color color) {
    if (rows.empty()) return;

    sf::VertexArray quads(sf::Quads, rows.size() * 4);

    for (std::size_t i = 0; i < rows.size(); ++i) {
        const float x1 = rows[i].first.x;
        const float x2 = rows[i].second.x + 1.0f;
        const float y = rows[i].first.y;
        sf::Vertex* quad = &quads[i * 4];

        quad[0].position = {x1, y};
        quad[1].position = {x2, y};
        quad[2].position = {x2, y + 1.0f};
        quad[3].position = {x1, y + 1.0f};

        for (int k = 0; k < 4; ++k)
            quad[k].color = color;
    }

    target.draw(quads);
}

// Small square marker used by the debug views.
inline void marker(sf::RenderTarget& target, Point centre,
                   float halfSize, sf::Color color) {
    plot(target,
         std::vector<Point>{Point{centre.x - halfSize, centre.y - halfSize}},
         color,
         halfSize * 2.0f);
}

}
