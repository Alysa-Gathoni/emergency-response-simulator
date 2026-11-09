#include "Mat3.h"
#include <cmath>

namespace {
constexpr float PI = 3.14159265358979323846f;
}

Mat3 Mat3::identity() {
    return {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
}

Mat3 Mat3::translation(float tx, float ty) {
    return {{{1, 0, tx}, {0, 1, ty}, {0, 0, 1}}};
}

Mat3 Mat3::rotation(float radians) {
    // Counter-clockwise in maths axes; because screen y points down this
    // turns clockwise on screen, matching atan2(dy, dx) headings.
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    return {{{c, -s, 0}, {s, c, 0}, {0, 0, 1}}};
}

Mat3 Mat3::rotationDegrees(float degrees) {
    return rotation(degrees * PI / 180.0f);
}

Mat3 Mat3::scaling(float sx, float sy) {
    return {{{sx, 0, 0}, {0, sy, 0}, {0, 0, 1}}};
}

Mat3 Mat3::aboutPivot(const Mat3& transform, Point pivot) {
    return translation(pivot.x, pivot.y) * transform * translation(-pivot.x, -pivot.y);
}

Mat3 Mat3::operator*(const Mat3& rhs) const {
    Mat3 r{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < 3; ++k)
                sum += m[i][k] * rhs.m[k][j];
            r.m[i][j] = sum;
        }
    return r;
}

Point Mat3::apply(Point p) const {
    // The bottom row is (0, 0, 1) for affine matrices, so w stays 1.
    return {
        m[0][0] * p.x + m[0][1] * p.y + m[0][2],
        m[1][0] * p.x + m[1][1] * p.y + m[1][2]
    };
}

std::vector<Point> Mat3::apply(const std::vector<Point>& points) const {
    std::vector<Point> result;
    result.reserve(points.size());
    for (const Point& p : points)
        result.push_back(apply(p));
    return result;
}

float Mat3::determinant() const {
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

Mat3 Mat3::inverse() const {
    // For an affine matrix [A t; 0 1] the inverse is [A^-1  -A^-1 t; 0 1].
    const float a = m[0][0], b = m[0][1], c = m[1][0], d = m[1][1];
    const float det = a * d - b * c;
    if (std::abs(det) < 1e-12f) return identity();

    const float ia = d / det, ib = -b / det, ic = -c / det, id = a / det;
    const float tx = m[0][2], ty = m[1][2];
    return {{
        {ia, ib, -(ia * tx + ib * ty)},
        {ic, id, -(ic * tx + id * ty)},
        {0, 0, 1}
    }};
}
