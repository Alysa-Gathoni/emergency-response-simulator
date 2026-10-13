#include "ClippingDemo.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include "../core/Color.h"
#include "../core/Raster.h"
#include <utility>

void ClippingDemo::draw(sf::RenderTarget& target,
                        const LineRenderer& lines,
                        bool enabled) const {
    if (!enabled) return;

    // --- Sutherland-Hodgman polygon clipping ---
    // A polygon intentionally extends beyond the right side of the viewport.
    const std::vector<Point> original = {
        {735, 205}, {855, 220}, {845, 330}, {755, 345}, {710, 275}
    };

    // Show original geometry faintly so the clipping operation is visible.
    Polygon originalPoly(original);
    originalPoly.drawOutline(target, lines, sf::Color(120,120,120), 1.0f);

    const auto clipped = Clipping::sutherlandHodgman(original, viewport_);
    Polygon clippedPoly(clipped);

    PolygonFill::draw(target, clipped, lines, sf::Color(75,115,105), false);
    clippedPoly.drawOutline(target, lines, Colors::Debug, 3.0f);
    clippedPoly.drawVertices(target, Colors::Debug);

    // --- Cohen-Sutherland line clipping ---
    // Segments crossing one or two viewport edges, one fully outside
    // (rejected) and one fully inside (accepted unchanged).
    const std::pair<Point, Point> segments[] = {
        {{20, 120},  {260, 10}},
        {{640, 690}, {890, 560}},
        {{40, 470},  {170, 695}},
        {{850, 20},  {890, 90}},
        {{600, 400}, {700, 470}}
    };

    for (const auto& [start, end] : segments) {
        lines.draw(target, start, end, sf::Color(120,120,120), 1.0f);

        Point a = start;
        Point b = end;
        if (!Clipping::cohenSutherland(a, b, viewport_)) continue;

        lines.draw(target, a, b, sf::Color(240,190,70), 3.0f);
        Raster::marker(target, a, 3.0f, sf::Color(240,190,70));
        Raster::marker(target, b, 3.0f, sf::Color(240,190,70));
    }

    // Draw clipping rectangle with the custom line renderer.
    lines.draw(target, {viewport_.left, viewport_.top},
               {viewport_.right, viewport_.top}, Colors::Debug, 2.0f);
    lines.draw(target, {viewport_.right, viewport_.top},
               {viewport_.right, viewport_.bottom}, Colors::Debug, 2.0f);
    lines.draw(target, {viewport_.right, viewport_.bottom},
               {viewport_.left, viewport_.bottom}, Colors::Debug, 2.0f);
    lines.draw(target, {viewport_.left, viewport_.bottom},
               {viewport_.left, viewport_.top}, Colors::Debug, 2.0f);
}
