#include "Check.h"
#include "simulation/Collision.h"

using namespace Collision;

namespace {

void boundsOfRotatedSquare() {
    // A square rotated 45 degrees: corners on the axes at distance 10.
    const AABB b = boundsOf({{0, -10}, {10, 0}, {0, 10}, {-10, 0}});
    CHECK_NEAR(b.minX, -10.0f, 1e-5f);
    CHECK_NEAR(b.maxX, 10.0f, 1e-5f);
    CHECK_NEAR(b.width(), 20.0f, 1e-5f);
}

void overlapCases() {
    const AABB a{0, 0, 10, 10};
    CHECK(overlaps(a, {5, 5, 15, 15}));      // partial overlap
    CHECK(overlaps(a, {2, 2, 4, 4}));        // fully inside
    CHECK(!overlaps(a, {20, 0, 30, 10}));    // apart on x
    CHECK(!overlaps(a, {0, 20, 10, 30}));    // apart on y
    CHECK(!overlaps(a, {10, 0, 20, 10}));    // touching edge is not overlap
}

void translationUsesShallowestAxis() {
    // a overlaps b by 2 on x and 8 on y -> push 2 along x (to the left).
    const Point push = minimumTranslation({0, 0, 10, 10}, {8, 1, 20, 9});
    CHECK_NEAR(push.x, -2.0f, 1e-5f);
    CHECK_NEAR(push.y, 0.0f, 1e-5f);

    // After applying the push the boxes only touch.
    const AABB moved{0 + push.x, 0, 10 + push.x, 10};
    CHECK(!overlaps(moved, {8, 1, 20, 9}));

    // No overlap -> no push.
    const Point none = minimumTranslation({0, 0, 1, 1}, {5, 5, 6, 6});
    CHECK(none.x == 0.0f && none.y == 0.0f);
}

void staysInsideWorld() {
    const Point fix = keepInside({-5, 10, 15, 30}, {0, 0, 100, 100});
    CHECK_NEAR(fix.x, 5.0f, 1e-5f);
    CHECK_NEAR(fix.y, 0.0f, 1e-5f);
}

}

int main() {
    boundsOfRotatedSquare();
    overlapCases();
    translationUsesShallowestAxis();
    staysInsideWorld();
    return check::failures();
}
