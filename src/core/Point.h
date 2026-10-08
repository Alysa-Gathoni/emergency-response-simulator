#pragma once

struct Point {
    float x{};
    float y{};

    Point() = default;
    Point(float xValue, float yValue) : x(xValue), y(yValue) {}
};
