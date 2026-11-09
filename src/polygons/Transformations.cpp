#include "Transformations.h"
#include "../core/Mat3.h"

// The original per-transform functions are kept as thin wrappers: each one
// builds the matching homogeneous matrix and applies it to every vertex.

namespace Transformations {

std::vector<Point> translate(const std::vector<Point>& vertices,
                             float dx, float dy) {
    return Mat3::translation(dx, dy).apply(vertices);
}

std::vector<Point> rotate(const std::vector<Point>& vertices,
                          float degrees, Point pivot) {
    return Mat3::aboutPivot(Mat3::rotationDegrees(degrees), pivot).apply(vertices);
}

std::vector<Point> scale(const std::vector<Point>& vertices,
                         float sx, float sy, Point pivot) {
    return Mat3::aboutPivot(Mat3::scaling(sx, sy), pivot).apply(vertices);
}

std::vector<Point> transform(const std::vector<Point>& vertices,
                             const Mat3& matrix) {
    return matrix.apply(vertices);
}

}
