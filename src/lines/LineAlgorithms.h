#pragma once
#include <vector>
#include "core/Point.h"

enum class LineAlgorithm { DDA, Bresenham };

class LineAlgorithms {
public:
    static std::vector<Point> dda(int x0, int y0, int x1, int y1);
    static std::vector<Point> bresenham(int x0, int y0, int x1, int y1);
};
