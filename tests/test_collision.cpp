#include "Check.h"
#include "simulation/Collision.h"
#include <cmath>
#include <vector>

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

// Rectangle of half-size (hx, hy) centred at c, rotated by `deg` degrees.
std::vector<Point> box(Point c, float hx, float hy, float deg) {
    const float a = deg * 3.14159265f / 180.0f, cs = std::cos(a), sn = std::sin(a);
    std::vector<Point> out;
    for (const Point& p : {Point{-hx, -hy}, Point{hx, -hy}, Point{hx, hy}, Point{-hx, hy}})
        out.push_back({c.x + p.x * cs - p.y * sn, c.y + p.x * sn + p.y * cs});
    return out;
}

void satAxisAligned() {
    // Same case as the AABB test: overlap 2 on x, 8 on y.
    const Contact c = sat(box({5, 5}, 5, 5, 0), box({14, 5}, 6, 4, 0));
    CHECK(c.hit);
    CHECK_NEAR(c.depth, 2.0f, 1e-4f);
    CHECK_NEAR(c.normal.x, -1.0f, 1e-4f);    // a is pushed left, out of b
    CHECK_NEAR(c.normal.y, 0.0f, 1e-4f);
    CHECK(!sat(box({0, 0}, 5, 5, 0), box({20, 0}, 5, 5, 0)).hit);
}

void satRotatedBoxes() {
    // Two boxes rotated 45 degrees, diagonal neighbours: their AABBs
    // overlap (broad phase says "maybe") but SAT finds the gap.
    const auto a = box({0, 0}, 10, 3, 45);
    const auto b = box({12, -2}, 10, 3, 45);
    CHECK(overlaps(boundsOf(a), boundsOf(b)));
    CHECK(!sat(a, b).hit);

    // Moved into each other along their short axis they do collide, and
    // pushing a by normal * depth separates them.
    const auto c = box({2, -2}, 10, 3, 45);
    const Contact hit = sat(a, c);
    CHECK(hit.hit);
    CHECK(hit.depth > 0.0f);
    CHECK_NEAR(std::hypot(hit.normal.x, hit.normal.y), 1.0f, 1e-4f);
    std::vector<Point> moved = a;
    for (Point& p : moved) { p.x += hit.normal.x * (hit.depth + 0.01f); p.y += hit.normal.y * (hit.depth + 0.01f); }
    CHECK(!sat(moved, c).hit);
}

void satTriangleAndWinding() {
    // Triangle vs box, and the result does not depend on the winding.
    const std::vector<Point> tri = {{0, 0}, {10, 0}, {0, 10}};
    const std::vector<Point> triReversed = {{0, 10}, {10, 0}, {0, 0}};
    const auto near = box({6, 6}, 2, 2, 0);    // the box corner (4,4) crosses the hypotenuse
    const auto far = box({8, 8}, 1.5f, 1.5f, 0);
    CHECK(sat(near, tri).hit);
    CHECK(sat(near, triReversed).hit);
    CHECK(!sat(far, tri).hit);
    CHECK_NEAR(sat(near, tri).depth, sat(near, triReversed).depth, 1e-4f);
}

}

int main() {
    boundsOfRotatedSquare();
    overlapCases();
    translationUsesShallowestAxis();
    staysInsideWorld();
    satAxisAligned();
    satRotatedBoxes();
    satTriangleAndWinding();
    return check::failures();
}
