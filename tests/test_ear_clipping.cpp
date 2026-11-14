#include "Check.h"
#include "polygons/Triangulation.h"
#include <cmath>
#include <vector>

namespace {

float area(const std::vector<Point>& p) { return std::abs(Triangulation::signedArea(p)); }

// Sum of the triangle areas must equal the polygon area: the triangles
// cover the polygon exactly once (no gaps, no overlaps, nothing outside).
float trianglesArea(const std::vector<Point>& polygon) {
    const auto tris = Triangulation::triangles(polygon);
    float total = 0.0f;
    for (std::size_t i = 0; i + 2 < tris.size(); i += 3)
        total += area({tris[i], tris[i + 1], tris[i + 2]});
    return total;
}

void shoelaceAndWinding() {
    const std::vector<Point> square = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
    CHECK_NEAR(Triangulation::signedArea(square), 100.0f, 1e-3f);    // clockwise on screen
    const std::vector<Point> reversed = {{0, 10}, {10, 10}, {10, 0}, {0, 0}};
    CHECK_NEAR(Triangulation::signedArea(reversed), -100.0f, 1e-3f);
    CHECK(Triangulation::insideTriangle({2, 2}, {0, 0}, {10, 0}, {0, 10}));
    CHECK(!Triangulation::insideTriangle({8, 8}, {0, 0}, {10, 0}, {0, 10}));
}

void convexPolygons() {
    const std::vector<Point> square = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
    CHECK(Triangulation::earClip(square).size() == 2);
    CHECK_NEAR(trianglesArea(square), 100.0f, 1e-2f);

    // Regular hexagon: n - 2 = 4 triangles.
    std::vector<Point> hex;
    for (int i = 0; i < 6; ++i) {
        const float a = static_cast<float>(i) * 3.14159265f / 3.0f;
        hex.push_back({50.0f + 20.0f * std::cos(a), 50.0f + 20.0f * std::sin(a)});
    }
    CHECK(Triangulation::earClip(hex).size() == 4);
    CHECK_NEAR(trianglesArea(hex), area(hex), 1e-2f);
}

void concavePolygons() {
    // L shape (one reflex corner), both windings.
    std::vector<Point> l = {{0, 0}, {60, 0}, {60, 40}, {100, 40}, {100, 100}, {0, 100}};
    CHECK(Triangulation::earClip(l).size() == 4);
    CHECK_NEAR(trianglesArea(l), area(l), 1e-2f);
    std::vector<Point> reversed(l.rbegin(), l.rend());
    CHECK(Triangulation::earClip(reversed).size() == 4);
    CHECK_NEAR(trianglesArea(reversed), area(l), 1e-2f);

    // No triangle may contain the reflex corner's notch: its centroid
    // must lie inside the polygon, never in the cut-away corner.
    const auto tris = Triangulation::triangles(l);
    for (std::size_t i = 0; i + 2 < tris.size(); i += 3) {
        const Point c{(tris[i].x + tris[i + 1].x + tris[i + 2].x) / 3.0f,
                      (tris[i].y + tris[i + 1].y + tris[i + 2].y) / 3.0f};
        CHECK(!(c.x > 60.0f && c.y < 40.0f));
    }

    // Star with five reflex vertices.
    std::vector<Point> star;
    for (int i = 0; i < 10; ++i) {
        const float r = i % 2 == 0 ? 50.0f : 20.0f;
        const float a = static_cast<float>(i) * 3.14159265f / 5.0f;
        star.push_back({100.0f + r * std::cos(a), 100.0f + r * std::sin(a)});
    }
    CHECK(Triangulation::earClip(star).size() == 8);
    CHECK_NEAR(trianglesArea(star), area(star), 0.5f);
}

void degenerateInput() {
    CHECK(Triangulation::earClip({}).empty());
    CHECK(Triangulation::earClip({{0, 0}, {5, 5}}).empty());
    // Collinear points and a repeated vertex are skipped, not triangulated.
    const std::vector<Point> withExtras = {{0, 0}, {5, 0}, {10, 0}, {10, 10}, {10, 10}, {0, 10}};
    CHECK_NEAR(trianglesArea(withExtras), 100.0f, 1e-2f);
    CHECK(Triangulation::earClip(withExtras).size() == 2);
    // A line has no area and gives no triangles.
    CHECK(Triangulation::earClip({{0, 0}, {5, 0}, {10, 0}}).empty());
}

}

int main() {
    shoelaceAndWinding();
    convexPolygons();
    concavePolygons();
    degenerateInput();
    return check::failures();
}
