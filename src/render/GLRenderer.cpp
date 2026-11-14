#include "GLRenderer.h"
#include <SFML/OpenGL.hpp>

void GLRenderer::toGlMatrix(const Mat3& m, float out[16]) {
    // | a b 0 tx |
    // | c d 0 ty |    stored column by column
    // | 0 0 1 0  |
    // | 0 0 0 1  |
    const float values[16] = {
        m.m[0][0], m.m[1][0], 0.0f, 0.0f,
        m.m[0][1], m.m[1][1], 0.0f, 0.0f,
        0.0f,      0.0f,      1.0f, 0.0f,
        m.m[0][2], m.m[1][2], 0.0f, 1.0f,
    };
    for (int i = 0; i < 16; ++i) out[i] = values[i];
}

void GLRenderer::beginFrame(Rgba clear) {
    target_->setActive(true);
    const sf::Vector2u size = target_->getSize();

    // Undo any state SFML left behind, then set up our own pipeline.
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, size.x, size.y, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(clear.r / 255.0f, clear.g / 255.0f, clear.b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    setView(Mat3::identity());
}

void GLRenderer::endFrame() {
    glFlush();
}

void GLRenderer::applyMatrix(const Mat3& matrix) {
    float m[16];
    toGlMatrix(matrix, m);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m);
}

void GLRenderer::drawPixels(const std::vector<Point>& pixels, Rgba c, float size) {
    if (pixels.empty()) return;
    glColor4ub(c.r, c.g, c.b, c.a);
    glBegin(GL_QUADS);
    for (const Point& p : pixels) {
        glVertex2f(p.x, p.y);
        glVertex2f(p.x + size, p.y);
        glVertex2f(p.x + size, p.y + size);
        glVertex2f(p.x, p.y + size);
    }
    glEnd();
}

void GLRenderer::drawSpans(const std::vector<std::pair<Point, Point>>& spans, Rgba c) {
    if (spans.empty()) return;
    glColor4ub(c.r, c.g, c.b, c.a);
    glBegin(GL_QUADS);
    for (const auto& [a, b] : spans) {
        const float x2 = b.x + 1.0f;
        glVertex2f(a.x, a.y);
        glVertex2f(x2, a.y);
        glVertex2f(x2, a.y + 1.0f);
        glVertex2f(a.x, a.y + 1.0f);
    }
    glEnd();
}

void GLRenderer::drawNativeLines(const std::vector<Point>& endpoints, Rgba c, float width) {
    glLineWidth(width);
    glColor4ub(c.r, c.g, c.b, c.a);
    glBegin(GL_LINES);
    for (const Point& p : endpoints) glVertex2f(p.x, p.y);
    glEnd();
    glLineWidth(1.0f);
}

void GLRenderer::drawNativeTriangles(const std::vector<Point>& vertices, Rgba c) {
    glColor4ub(c.r, c.g, c.b, c.a);
    glBegin(GL_TRIANGLES);
    for (const Point& p : vertices) glVertex2f(p.x, p.y);
    glEnd();
}

bool GLRenderer::beginStatic(unsigned key) {
    // Replay: one call sends the whole recorded city to the driver, which
    // already holds the vertex data, instead of thousands of glVertex calls.
    if (list_ != 0 && key == listKey_ && target_ == listTarget_) {
        glCallList(list_);
        return false;
    }

    // (Re)record. The list stores the vertices and colours in world
    // coordinates; the view matrix is loaded before glCallList each frame,
    // so the camera can move without rebuilding it.
    if (list_ == 0) list_ = glGenLists(1);
    listKey_ = key;
    listTarget_ = target_;
    recording_ = true;
    ++rebuilds_;
    glNewList(list_, GL_COMPILE_AND_EXECUTE);
    return true;
}

void GLRenderer::endStatic() {
    if (!recording_) return;
    glEndList();
    recording_ = false;
}
