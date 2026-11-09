#pragma once
#include <vector>
#include "../core/Point.h"

struct BezierSegment {
    Point p0;
    Point p1;
    Point p2;
    Point p3;
};

// A route made of connected cubic Bezier segments.
// Positions are looked up by arc length, so a constant change in progress
// moves the same distance along the path regardless of segment length.
class SplinePath {
public:
    // Builds a path that passes through every knot (Catmull-Rom spline).
    static SplinePath fromCatmullRom(const std::vector<Point>& knots,
                                     bool closed);

    void addSegment(const BezierSegment& segment);
    void setSegments(std::vector<BezierSegment> segments);

    // progress in [0, 1) maps to [0, length()) along the path.
    Point evaluate(float progress) const;
    Point tangent(float progress) const;
    float length() const;

    // Same lookups by distance travelled along the path (wraps around).
    Point pointAtDistance(float distance) const;
    // Unit tangent (direction of travel) at a distance along the path.
    Point directionAtDistance(float distance) const;

    std::vector<Point> sampledPoints(int segmentsPerCurve = 60) const;
    const std::vector<BezierSegment>& segments() const;
    const std::vector<Point>& knots() const;

    bool empty() const;

private:
    struct ArcSample {
        float length;
        std::size_t segment;
        float t;
    };

    static constexpr int SamplesPerSegment = 128;

    std::vector<BezierSegment> segments_;
    std::vector<Point> knots_;
    std::vector<ArcSample> arcTable_;

    void rebuildArcTable();
};
