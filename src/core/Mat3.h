#pragma once
#include <vector>
#include "Point.h"

// 3x3 matrix for 2D homogeneous coordinates.
//
// A point (x, y) is treated as the column vector (x, y, 1), so translation
// becomes a matrix product like rotation and scaling:
//
//   | a b tx |   | x |   | a*x + b*y + tx |
//   | c d ty | * | y | = | c*x + d*y + ty |
//   | 0 0 1  |   | 1 |   |       1        |
//
// Composition order: matrices apply right to left. For an object placed in
// the world we use
//
//   world = T(position) * R(heading) * S(size) * local
//
// i.e. scale in local space first, then rotate, then move into place.
// Transforming about a pivot p is T(p) * M * T(-p).
struct Mat3 {
    float m[3][3];

    static Mat3 identity();
    static Mat3 translation(float tx, float ty);
    static Mat3 rotation(float radians);
    static Mat3 rotationDegrees(float degrees);
    static Mat3 scaling(float sx, float sy);

    // T(pivot) * transform * T(-pivot)
    static Mat3 aboutPivot(const Mat3& transform, Point pivot);

    Mat3 operator*(const Mat3& rhs) const;

    Point apply(Point p) const;
    std::vector<Point> apply(const std::vector<Point>& points) const;

    // Inverse of an affine matrix (only the 2x2 part needs a real inverse).
    // Returns identity if the matrix is singular.
    Mat3 inverse() const;

    float determinant() const;
};
