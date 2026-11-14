#pragma once
#include "render/IRenderer.h"
#include "../lines/LineRenderer.h"
#include "../rasterization/Clipping.h"

// C key: the camera's clip window is shrunk so the clipping that normally
// happens at the edge of the screen becomes visible in the middle of it.
// Every road, building and vehicle outside the yellow frame is cut by our
// Cohen-Sutherland (lines) and Sutherland-Hodgman (polygons) code before
// rasterisation; the frame shows where.
class ClippingDemo {
public:
    static constexpr float Inset = 150.0f;   // px from each window edge

    void draw(IRenderer& target, const LineRenderer& lines,
              bool enabled, const ClipRect& window) const;
};
