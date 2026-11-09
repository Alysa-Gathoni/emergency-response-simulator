#include "Check.h"
#include "core/Mat3.h"

namespace {

bool samePoint(Point a, Point b, float eps = 1e-3f) {
    return check::near(a.x, b.x, eps) && check::near(a.y, b.y, eps);
}

void identityLeavesPointsAlone() {
    CHECK(samePoint(Mat3::identity().apply({3, -4}), {3, -4}));
}

void rotate90() {
    // (1, 0) rotated by +90 degrees lands on (0, 1).
    CHECK(samePoint(Mat3::rotationDegrees(90).apply({1, 0}), {0, 1}));
    CHECK(samePoint(Mat3::rotationDegrees(90).apply({0, 1}), {-1, 0}));
}

void translationAndScaling() {
    CHECK(samePoint(Mat3::translation(5, -2).apply({1, 1}), {6, -1}));
    CHECK(samePoint(Mat3::scaling(2, 3).apply({1, 1}), {2, 3}));
}

void inverseRestoresOriginal() {
    const Mat3 m = Mat3::translation(40, 10) *
                   Mat3::rotationDegrees(33) *
                   Mat3::scaling(2, 0.5f);
    const Point p{7, -3};
    CHECK(samePoint(m.inverse().apply(m.apply(p)), p));

    const Mat3 shouldBeIdentity = m * m.inverse();
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            CHECK_NEAR(shouldBeIdentity.m[i][j], i == j ? 1.0f : 0.0f, 1e-4f);
}

void compositionOrderMatters() {
    const Point p{1, 0};
    // Rotate then translate: (1,0) -> (0,1) -> (10,1)
    const Point a = (Mat3::translation(10, 0) * Mat3::rotationDegrees(90)).apply(p);
    // Translate then rotate: (1,0) -> (11,0) -> (0,11)
    const Point b = (Mat3::rotationDegrees(90) * Mat3::translation(10, 0)).apply(p);
    CHECK(samePoint(a, {10, 1}));
    CHECK(samePoint(b, {0, 11}));
}

void pivotRotationKeepsPivotFixed() {
    const Point pivot{50, 20};
    const Mat3 m = Mat3::aboutPivot(Mat3::rotationDegrees(45), pivot);
    CHECK(samePoint(m.apply(pivot), pivot));
    // A point 10 to the right of the pivot ends up 10 below it after 90 deg.
    const Mat3 q = Mat3::aboutPivot(Mat3::rotationDegrees(90), pivot);
    CHECK(samePoint(q.apply({60, 20}), {50, 30}));
}

void determinantOfScale() {
    CHECK_NEAR(Mat3::scaling(2, 3).determinant(), 6.0f, 1e-5f);
    CHECK_NEAR(Mat3::rotationDegrees(70).determinant(), 1.0f, 1e-5f);
}

}

int main() {
    identityLeavesPointsAlone();
    rotate90();
    translationAndScaling();
    inverseRestoresOriginal();
    compositionOrderMatters();
    pivotRotationKeepsPivotFixed();
    determinantOfScale();
    return check::failures();
}
