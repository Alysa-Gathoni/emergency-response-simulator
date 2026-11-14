#pragma once
#include <SFML/Graphics.hpp>
#include "IRenderer.h"

// Software path: every vertex is transformed on the CPU with our Mat3,
// then all quads of one call are handed to SFML in a single batch. SFML
// only copies them to the window; it computes no geometry.
class SoftwareRenderer : public IRenderer {
public:
    void setTarget(sf::RenderTarget& target) { target_ = &target; }

    const char* name() const override { return "SOFTWARE"; }
    void beginFrame(Rgba clear) override;
    void endFrame() override {}
    bool hardwareTransforms() const override { return false; }

    void drawPixels(const std::vector<Point>& pixels, Rgba colour, float size) override;
    void drawSpans(const std::vector<std::pair<Point, Point>>& spans, Rgba colour) override;
    void drawNativeLines(const std::vector<Point>& endpoints, Rgba colour, float width) override;
    void drawNativeTriangles(const std::vector<Point>& vertices, Rgba colour) override;

protected:
    void applyMatrix(const Mat3& matrix) override { matrix_ = matrix; }

private:
    sf::RenderTarget* target_{nullptr};
    Mat3 matrix_ = Mat3::identity();

    void quad(sf::VertexArray& out, std::size_t at, Point p0, Point p1, Point p2, Point p3,
              sf::Color colour) const;
};
