#include "Check.h"
#include "core/Letterbox.h"

namespace {

void sameSizeIsIdentity() {
    const Letterbox b = Letterbox::fit(1100, 720, 1100, 720);
    CHECK_NEAR(b.scale, 1.0f, 1e-6f);
    CHECK_NEAR(b.offsetX, 0.0f, 1e-6f);
    CHECK_NEAR(b.offsetY, 0.0f, 1e-6f);
    const Point p = b.toLogical({300, 200});
    CHECK_NEAR(p.x, 300.0f, 1e-4f);
    CHECK_NEAR(p.y, 200.0f, 1e-4f);
}

void widerWindowGetsSideBars() {
    // 1920 x 1080: scale = min(1.745, 1.5) = 1.5, drawn area 1650 x 1080.
    const Letterbox b = Letterbox::fit(1920, 1080, 1100, 720);
    CHECK_NEAR(b.scale, 1.5f, 1e-5f);
    CHECK_NEAR(b.width, 1650.0f, 1e-3f);
    CHECK_NEAR(b.height, 1080.0f, 1e-3f);
    CHECK_NEAR(b.offsetX, 135.0f, 1e-3f);
    CHECK_NEAR(b.offsetY, 0.0f, 1e-3f);
    // The centre of the window is the centre of the logical screen.
    const Point c = b.toLogical({960, 540});
    CHECK_NEAR(c.x, 550.0f, 1e-3f);
    CHECK_NEAR(c.y, 360.0f, 1e-3f);
    // A click on the left bar maps to a negative x (outside every panel).
    CHECK(b.toLogical({10, 540}).x < 0.0f);
}

void tallWindowGetsTopAndBottomBars() {
    const Letterbox b = Letterbox::fit(550, 1000, 1100, 720);
    CHECK_NEAR(b.scale, 0.5f, 1e-6f);
    CHECK_NEAR(b.offsetX, 0.0f, 1e-4f);
    CHECK_NEAR(b.offsetY, (1000.0f - 360.0f) * 0.5f, 1e-3f);
    const Point corner = b.toLogical({550, b.offsetY + 360.0f});
    CHECK_NEAR(corner.x, 1100.0f, 1e-3f);
    CHECK_NEAR(corner.y, 720.0f, 1e-3f);
}

void minimisedWindowDrawsNothing() {
    const Letterbox b = Letterbox::fit(0, 0, 1100, 720);
    CHECK(!b.visible());
    const Point p = b.toLogical({10, 10});       // no division by zero
    CHECK(p.x == 0.0f && p.y == 0.0f);
}

}

int main() {
    sameSizeIsIdentity();
    widerWindowGetsSideBars();
    tallWindowGetsTopAndBottomBars();
    minimisedWindowDrawsNothing();
    return check::failures();
}
