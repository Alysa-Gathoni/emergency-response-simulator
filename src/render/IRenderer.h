#pragma once
#include <utility>
#include <vector>
#include "../core/Mat3.h"
#include "../core/Point.h"
#include "../core/Rgba.h"

// The only interface the scene uses to put anything on screen.
//
//   scene modules (city, vehicles, HUD, ...)      pure C++ algorithms
//        | pixels, scan-line spans, segments       (lines, fill, circles,
//        v                                          curves, clipping)
//   IRenderer  <- view matrix + model transform stack
//        |
//        +-- SoftwareRenderer: transforms on the CPU with Mat3, hands the
//        |   finished pixel quads to SFML in one batch
//        +-- GLRenderer: legacy OpenGL (immediate mode), glOrtho
//            projection, glLoadMatrixf for the view/model matrices
//
// Our algorithms still compute the geometry; a renderer only submits it.
// Two "native" calls exist so the demo can compare our output with what
// the graphics API draws by itself (GL_LINES, GL_TRIANGLES).
class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual const char* name() const = 0;
    virtual void beginFrame(Rgba clear) = 0;
    virtual void endFrame() = 0;

    // True when the model matrix stack is applied by graphics hardware.
    virtual bool hardwareTransforms() const = 0;

    // World -> screen matrix for everything drawn next (clears the stack).
    void setView(const Mat3& worldToScreen) {
        view_ = worldToScreen;
        stack_.clear();
        applyMatrix(current());
    }

    // Model transforms, like glPushMatrix / glMultMatrix / glPopMatrix.
    void pushTransform(const Mat3& model) {
        stack_.push_back(current() * model);
        applyMatrix(current());
    }
    void popTransform() {
        if (!stack_.empty()) stack_.pop_back();
        applyMatrix(current());
    }

    const Mat3& view() const { return view_; }
    Mat3 current() const { return stack_.empty() ? view_ : stack_.back(); }

    // True while a model matrix is pushed; model() maps the coordinates
    // being drawn to world space (view^-1 * current).
    bool hasModel() const { return !stack_.empty(); }
    Mat3 model() const { return view_.inverse() * current(); }

    // Square pixels of side `size` produced by our algorithms.
    virtual void drawPixels(const std::vector<Point>& pixels, Rgba colour, float size) = 0;

    // Horizontal scan-line spans: x1..x2 inclusive on row y.
    virtual void drawSpans(const std::vector<std::pair<Point, Point>>& spans, Rgba colour) = 0;

    // Comparison mode only: segments (pairs of end points) and triangles
    // rasterised by the graphics API instead of by our algorithms.
    virtual void drawNativeLines(const std::vector<Point>& endpoints, Rgba colour, float width) = 0;
    virtual void drawNativeTriangles(const std::vector<Point>& vertices, Rgba colour) = 0;

protected:
    // Called whenever the current matrix changes.
    virtual void applyMatrix(const Mat3& matrix) = 0;

private:
    Mat3 view_ = Mat3::identity();
    std::vector<Mat3> stack_;
};
