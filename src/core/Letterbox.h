#pragma once
#include <algorithm>
#include "Point.h"

// Fitting the fixed logical screen (1100 x 720) into a window of any size.
//
//   scale  = min(windowW / logicalW, windowH / logicalH)
//   offset = (window - logical * scale) / 2
//
// One uniform scale keeps the aspect ratio; the unused strips (left/right
// or top/bottom) stay black. Every renderer draws in logical coordinates,
// and mouse positions are mapped back with the inverse:
//   logical = (window - offset) / scale
struct Letterbox {
    float scale{1.0f};
    float offsetX{0.0f};
    float offsetY{0.0f};
    float width{0.0f};    // of the drawn area, window pixels
    float height{0.0f};

    static Letterbox fit(float windowW, float windowH, float logicalW, float logicalH) {
        Letterbox b;
        if (windowW <= 0.0f || windowH <= 0.0f || logicalW <= 0.0f || logicalH <= 0.0f) {
            b.scale = 0.0f;    // minimised: nothing to draw
            return b;
        }
        b.scale = std::min(windowW / logicalW, windowH / logicalH);
        b.width = logicalW * b.scale;
        b.height = logicalH * b.scale;
        b.offsetX = (windowW - b.width) * 0.5f;
        b.offsetY = (windowH - b.height) * 0.5f;
        return b;
    }

    bool visible() const { return scale > 0.0f; }

    Point toLogical(Point window) const {
        if (scale <= 0.0f) return {0.0f, 0.0f};
        return {(window.x - offsetX) / scale, (window.y - offsetY) / scale};
    }
};
