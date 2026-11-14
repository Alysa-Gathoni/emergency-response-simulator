#include "ClippingDemo.h"
#include "../core/Raster.h"

void ClippingDemo::draw(IRenderer& target, const LineRenderer& lines,
                        bool enabled, const ClipRect& w) const {
    if (!enabled) return;
    const Rgba frame(240, 190, 70);
    const Point c[] = {{w.left, w.top}, {w.right, w.top}, {w.right, w.bottom}, {w.left, w.bottom}};
    for (int i = 0; i < 4; ++i) {
        lines.draw(target, c[i], c[(i + 1) % 4], frame, 2.0f);
        Raster::marker(target, c[i], 4.0f, frame);
    }
}
