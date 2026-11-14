#include "VehicleModels.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include "../shapes/CircleEllipse.h"
#include <cmath>
#include <vector>

namespace {

constexpr float PI = 3.14159265358979323846f;

const Rgba CarColours[] = {
    {70, 135, 205}, {196, 64, 58}, {190, 194, 200}, {64, 150, 112}, {230, 230, 226}
};
const Rgba MatatuColours[] = {
    {242, 196, 48}, {120, 70, 170}, {40, 170, 120}
};
const Rgba RiderColours[] = {
    {230, 120, 40}, {40, 120, 230}, {220, 220, 60}
};
const Rgba Glass(40, 52, 68);
const Rgba Tyre(22, 22, 24);
const Rgba Outline(20, 24, 30);
const Rgba Amber(255, 170, 40);

Rgba shade(Rgba c, float k) {
    return Rgba(static_cast<std::uint8_t>(c.r * k), static_cast<std::uint8_t>(c.g * k),
                     static_cast<std::uint8_t>(c.b * k));
}

std::vector<Point> rect(float l, float t, float r, float b) {
    return {{l, t}, {r, t}, {r, b}, {l, b}};
}

// Ellipse as a polygon in local space. Unlike the midpoint ellipse (which
// is axis aligned), the vertices can go through any matrix, so the shape
// rotates with the vehicle before the scan-line fill.
std::vector<Point> ellipsePolygon(float cx, float cy, float rx, float ry, int n = 14) {
    std::vector<Point> pts;
    for (int i = 0; i < n; ++i) {
        const float a = 2.0f * PI * i / n;
        pts.push_back({cx + rx * std::cos(a), cy + ry * std::sin(a)});
    }
    return pts;
}

struct Painter {
    IRenderer& target;
    const LineRenderer& lines;
    const Mat3& world;

    void fill(const std::vector<Point>& local, Rgba c) const {
        PolygonFill::draw(target, world.apply(local), lines, c, false);
    }
    void outline(const std::vector<Point>& local, Rgba c) const {
        Polygon(world.apply(local)).drawOutline(target, lines, c, 1.0f);
    }
    void line(Point a, Point b, Rgba c, float w = 1.0f) const {
        lines.draw(target, world.apply(a), world.apply(b), c, w);
    }
    void disc(Point p, int r, Rgba c) const {
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(world.apply(p), r),
                                    c, 1.0f);
    }
};

void lamps(const Painter& p, const VehiclePose& pose, float hl, float hw) {
    // Headlights, brake lights and indicators at the four corners.
    p.fill(rect(hl - 2, -hw + 1, hl, -hw + 4), Rgba(250, 240, 190));
    p.fill(rect(hl - 2, hw - 4, hl, hw - 1), Rgba(250, 240, 190));
    const Rgba brake = pose.braking ? Rgba(255, 45, 35) : Rgba(110, 25, 25);
    p.fill(rect(-hl, -hw + 1, -hl + 2, -hw + 4), brake);
    p.fill(rect(-hl, hw - 4, -hl + 2, hw - 1), brake);

    if (pose.indicator != 0 && pose.blinkOn) {
        // Local +y is the vehicle's right-hand side.
        const float side = pose.indicator > 0 ? hw - 1 : -hw + 1;
        p.disc({hl - 1, side}, 2, Amber);
        p.disc({-hl + 1, side}, 2, Amber);
    }
}

void drawCar(const Painter& p, const VehiclePose& pose) {
    const float hl = 16, hw = 9;
    const Rgba body = CarColours[pose.palette % 5];
    for (float x : {-10.0f, 10.0f})
        for (float y : {-hw - 1.0f, hw - 2.0f})
            p.fill(rect(x - 4, y, x + 4, y + 3), Tyre);

    p.fill(rect(-hl, -hw, hl, hw), body);
    p.outline(rect(-hl, -hw, hl, hw), Outline);
    p.fill(rect(-9, -7, 6, 7), shade(body, 0.82f));                // roof
    p.fill({{6, -7}, {11, -8}, {11, 8}, {6, 7}}, Glass);           // windscreen
    p.fill(rect(-13, -7, -10, 7), Glass);                          // rear window
    lamps(p, pose, hl, hw);
}

void drawMatatu(const Painter& p, const VehiclePose& pose) {
    const float hl = 30, hw = 12;
    const Rgba body = MatatuColours[pose.palette % 3];
    for (float x : {-20.0f, 19.0f})
        for (float y : {-hw - 1.0f, hw - 2.0f})
            p.fill(rect(x - 5, y, x + 5, y + 3), Tyre);

    p.fill(rect(-hl, -hw, hl, hw), body);
    p.outline(rect(-hl, -hw, hl, hw), Outline);

    // Side windows: a row of small panes along both sides.
    for (float x = -24.0f; x < 18.0f; x += 8.0f) {
        p.fill(rect(x, -hw + 1, x + 6, -hw + 3), Glass);
        p.fill(rect(x, hw - 3, x + 6, hw - 1), Glass);
    }
    // Livery stripe, roof rack bars and the illuminated route sign.
    p.line({-hl + 3, -hw + 5}, {hl - 8, -hw + 5}, Rgba(200, 40, 60), 2.0f);
    p.line({-hl + 3, hw - 6}, {hl - 8, hw - 6}, Rgba(200, 40, 60), 2.0f);
    for (float x = -22.0f; x <= -6.0f; x += 4.0f)
        p.line({x, -5}, {x, 5}, shade(body, 0.6f));
    p.fill(rect(2, -5, 14, 5), Rgba(250, 250, 250));
    p.line({4, -2}, {12, -2}, Rgba(40, 40, 40));
    p.line({4, 2}, {10, 2}, Rgba(40, 40, 40));
    p.fill({{hl - 7, -hw + 2}, {hl - 3, -hw + 3}, {hl - 3, hw - 3}, {hl - 7, hw - 2}}, Glass);
    lamps(p, pose, hl, hw);
}

void drawMotorcycle(const Painter& p, const VehiclePose& pose) {
    const float hl = 13;
    p.fill(rect(hl - 7, -1.5f, hl, 1.5f), Tyre);           // front wheel
    p.fill(rect(-hl, -1.5f, -hl + 7, 1.5f), Tyre);         // rear wheel
    p.fill(ellipsePolygon(0, 0, 8, 3), Rgba(60, 60, 66));
    p.line({hl - 6, -6}, {hl - 6, 6}, Rgba(160, 160, 160));   // handlebar
    // Rider: shoulders are a rotated ellipse, helmet a disc on top.
    p.fill(ellipsePolygon(-1, 0, 3, 6), RiderColours[pose.palette % 3]);
    p.disc({0, 0}, 3, shade(RiderColours[pose.palette % 3], 0.7f));
    p.fill(rect(hl - 1, -1, hl, 1), Rgba(250, 240, 190));
    p.fill(rect(-hl, -1, -hl + 1, 1),
           pose.braking ? Rgba(255, 45, 35) : Rgba(110, 25, 25));
}

}

namespace VehicleModels {

void draw(IRenderer& target, const LineRenderer& lines, const VehiclePose& pose) {
    // OpenGL: one glLoadMatrix-style push of T(position) * R(heading) and
    // the model is rasterised in local space. Software: every vertex goes
    // through the same matrix on the CPU before the scan-line fill.
    const bool hardware = target.hardwareTransforms();
    const Mat3 identity = Mat3::identity();
    if (hardware) target.pushTransform(pose.world);

    const Painter painter{target, lines, hardware ? identity : pose.world};
    switch (pose.type) {
        case VehicleType::Car:        drawCar(painter, pose); break;
        case VehicleType::Matatu:     drawMatatu(painter, pose); break;
        case VehicleType::Motorcycle: drawMotorcycle(painter, pose); break;
    }

    if (hardware) target.popTransform();
}

}
