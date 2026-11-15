#include "SoftwareRenderer.h"
#include "../core/Letterbox.h"

namespace {
sf::Color toSf(Rgba c) { return sf::Color(c.r, c.g, c.b, c.a); }
}

void SoftwareRenderer::beginFrame(Rgba clear) {
    // OpenGL may have drawn the previous frame into the same context.
    target_->resetGLStates();
    target_->setView(target_->getDefaultView());
    target_->clear(sf::Color::Black);

    // Letterbox: an SFML view of the logical screen with a viewport (in
    // fractions of the window) on the fitted area.
    const sf::Vector2u size = target_->getSize();
    const float lw = logicalWidth(static_cast<float>(size.x));
    const float lh = logicalHeight(static_cast<float>(size.y));
    const Letterbox box = Letterbox::fit(static_cast<float>(size.x), static_cast<float>(size.y), lw, lh);
    sf::View view(sf::FloatRect(0.0f, 0.0f, lw, lh));
    if (box.visible())
        view.setViewport(sf::FloatRect(box.offsetX / size.x, box.offsetY / size.y,
                                       box.width / size.x, box.height / size.y));
    target_->setView(view);

    // Background of the logical area (clear() ignores the viewport).
    matrix_ = Mat3::identity();
    sf::VertexArray background(sf::Quads, 4);
    quad(background, 0, {0, 0}, {lw, 0}, {lw, lh}, {0, lh}, toSf(clear));
    target_->draw(background);
    setView(Mat3::identity());
}

void SoftwareRenderer::quad(sf::VertexArray& out, std::size_t at,
                            Point p0, Point p1, Point p2, Point p3, sf::Color colour) const {
    const Point q[4] = {matrix_.apply(p0), matrix_.apply(p1),
                        matrix_.apply(p2), matrix_.apply(p3)};
    for (int k = 0; k < 4; ++k) {
        out[at + k].position = {q[k].x, q[k].y};
        out[at + k].color = colour;
    }
}

void SoftwareRenderer::drawPixels(const std::vector<Point>& pixels, Rgba colour, float size) {
    if (pixels.empty()) return;
    sf::VertexArray quads(sf::Quads, pixels.size() * 4);
    const sf::Color c = toSf(colour);
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        const float x = pixels[i].x, y = pixels[i].y;
        quad(quads, i * 4, {x, y}, {x + size, y}, {x + size, y + size}, {x, y + size}, c);
    }
    target_->draw(quads);
}

void SoftwareRenderer::drawSpans(const std::vector<std::pair<Point, Point>>& spans, Rgba colour) {
    if (spans.empty()) return;
    sf::VertexArray quads(sf::Quads, spans.size() * 4);
    const sf::Color c = toSf(colour);
    for (std::size_t i = 0; i < spans.size(); ++i) {
        const float x1 = spans[i].first.x;
        const float x2 = spans[i].second.x + 1.0f;
        const float y = spans[i].first.y;
        quad(quads, i * 4, {x1, y}, {x2, y}, {x2, y + 1.0f}, {x1, y + 1.0f}, c);
    }
    target_->draw(quads);
}

void SoftwareRenderer::drawNativeLines(const std::vector<Point>& endpoints, Rgba colour, float) {
    sf::VertexArray lines(sf::Lines, endpoints.size());
    for (std::size_t i = 0; i < endpoints.size(); ++i) {
        const Point p = matrix_.apply(endpoints[i]);
        lines[i] = sf::Vertex({p.x, p.y}, toSf(colour));
    }
    target_->draw(lines);
}

void SoftwareRenderer::drawNativeTriangles(const std::vector<Point>& vertices, Rgba colour) {
    sf::VertexArray tris(sf::Triangles, vertices.size());
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const Point p = matrix_.apply(vertices[i]);
        tris[i] = sf::Vertex({p.x, p.y}, toSf(colour));
    }
    target_->draw(tris);
}
