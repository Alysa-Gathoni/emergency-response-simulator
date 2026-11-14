#pragma once
#include "render/IRenderer.h"
#include <vector>
#include "LineAlgorithms.h"
#include "../rasterization/Clipping.h"

// Bresenham and DDA are our algorithms. Native hands the end points to the
// graphics API (GL_LINES) so the demo can compare the two side by side.
enum class LineMode { Bresenham, DDA, Native };

class LineRenderer {
public:
    explicit LineRenderer(LineMode mode = LineMode::Bresenham);

    void setMode(LineMode mode);
    void nextMode();                     // L key: Bresenham -> DDA -> Native
    LineMode getMode() const;
    void setDebug(bool enabled);
    bool isDebug() const;

    // Clip window (world coordinates): segments are cut to it with
    // Cohen-Sutherland and polygons with Sutherland-Hodgman before they are
    // rasterised, so nothing outside the camera view costs any work.
    void setClip(const ClipRect& rect);
    void clearClip();
    const ClipRect* clip() const;

    // Pixels of the segment using the currently selected algorithm.
    std::vector<Point> rasterize(Point start, Point end) const;

    void draw(IRenderer& target, Point start, Point end,
              Rgba color, float pixelSize = 2.0f) const;

private:
    LineMode mode_;
    bool debug_{false};
    bool clipping_{false};
    ClipRect clip_{};
};
