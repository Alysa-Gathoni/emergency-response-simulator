#pragma once
#include <SFML/Graphics.hpp>
#include "IRenderer.h"

// Legacy (fixed-function) OpenGL backend.
//
//   projection  glOrtho(0, width, height, 0, -1, 1)   2D, y down
//   modelview   glLoadMatrixf(current matrix)         view * model stack,
//                                                     Mat3 widened to 4x4
//   geometry    glBegin(GL_QUADS) ... glEnd()        our pixels and spans
//               glBegin(GL_LINES / GL_TRIANGLES)     comparison mode only
//   blending    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  (night tint, glows)
//
// SFML is only used to create the window and its OpenGL context; when this
// renderer is active SFML draws nothing.
class GLRenderer : public IRenderer {
public:
    void setTarget(sf::RenderTarget& target) { target_ = &target; }

    const char* name() const override { return "OPENGL"; }
    void beginFrame(Rgba clear) override;
    void endFrame() override;
    bool hardwareTransforms() const override { return true; }

    void drawPixels(const std::vector<Point>& pixels, Rgba colour, float size) override;
    void drawSpans(const std::vector<std::pair<Point, Point>>& spans, Rgba colour) override;
    void drawNativeLines(const std::vector<Point>& endpoints, Rgba colour, float width) override;
    void drawNativeTriangles(const std::vector<Point>& vertices, Rgba colour) override;

    // Column-major 4x4 equivalent of a 2D affine Mat3 (z untouched).
    static void toGlMatrix(const Mat3& m, float out[16]);

protected:
    void applyMatrix(const Mat3& matrix) override;

private:
    sf::RenderTarget* target_{nullptr};
};
