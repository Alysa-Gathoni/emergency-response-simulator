#pragma once
#include "render/IRenderer.h"
#include <vector>
#include "../core/Mat3.h"
#include "../lines/LineRenderer.h"
#include "../rasterization/Clipping.h"

class City;

// The whole district in a corner of the screen (M toggles it).
//
// Window-to-viewport transformation: the world window (the map) is mapped
// onto the viewport (a screen rectangle) with
//
//     M = T(viewport centre) * S(s, s) * T(-window centre)
//     s = min(viewport width / window width, viewport height / window height)
//
// One uniform scale keeps the aspect ratio, so circles stay round and the
// map is centred with a margin on the longer side. Every map point goes
// through M (computed once) and is then drawn in screen space by our own
// line and fill algorithms, clipped to the viewport.
class Minimap {
public:
    static constexpr float Width = 226.0f;
    static constexpr float Height = 156.0f;

    static Mat3 windowToViewport(const ClipRect& window, const ClipRect& viewport);

    // What changes every frame.
    struct Frame {
        Point ambulance;
        float headingDegrees{0.0f};
        std::vector<Point> route;
        Rgba routeColour;
        bool showTarget{false};
        Point target;
        std::vector<Point> traffic;
        ClipRect cameraView{};      // world rectangle visible on screen
        float time{0.0f};           // for the pulsing target
    };

    Minimap(const City& city, ClipRect viewport);

    void toggle() { visible_ = !visible_; }
    bool visible() const { return visible_; }
    const ClipRect& viewport() const { return viewport_; }
    const Mat3& transform() const { return transform_; }

    void draw(IRenderer& target, LineRenderer& lines, const Frame& frame) const;

private:
    const City& city_;
    ClipRect viewport_;
    Mat3 transform_;
    bool visible_{true};
};
