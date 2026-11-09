#include "City.h"
#include "../core/Color.h"
#include "../rasterization/PolygonFill.h"

namespace {

std::vector<Point> rectPoints(float l, float t, float r, float b) {
    return {{l, t}, {r, t}, {r, b}, {l, b}};
}

std::vector<Point> rectPoints(const Rect& r) {
    return rectPoints(r.left, r.top, r.right, r.bottom);
}

const sf::Color Pavement(58, 64, 72);
const sf::Color Grass(46, 84, 54);
const sf::Color Zebra(225, 225, 225);

}

City::City(const RoadNetwork& network) : network_(network) {
    // Buildings sit inside the blocks left between the roads, set back
    // from the kerb by a 14 px pavement.
    const float rects[][4] = {
        {14, 14, 105, 116},   {119, 14, 216, 116},
        {384, 14, 490, 116},  {504, 14, 600, 90},   {614, 14, 716, 116},
        {884, 14, 975, 116},
        {14, 284, 216, 350},  {14, 364, 108, 436},  {122, 364, 216, 436},
        {634, 284, 716, 436},
        {884, 284, 1086, 354}, {884, 368, 1086, 436},
        {384, 604, 540, 706},  {554, 604, 716, 706},
        {884, 604, 975, 706},  {989, 604, 1086, 706},
    };
    for (const auto& r : rects)
        buildings_.emplace_back(rectPoints(r[0], r[1], r[2], r[3]));

    hospital_ = Polygon(rectPoints(384, 284, 620, 436));

    parks_ = {
        {14, 604, 216, 706},
        {989, 14, 1086, 116},
    };
}

const std::vector<Polygon>& City::buildings() const { return buildings_; }
const Polygon& City::hospital() const { return hospital_; }
const std::vector<Rect>& City::parks() const { return parks_; }

void City::drawRoads(sf::RenderTarget& target, const LineRenderer& lines) const {
    // Road surfaces are rectangles filled with our scan-line algorithm.
    for (const Road& road : network_.roads()) {
        const float lo = road.centre - RoadNetwork::HalfWidth;
        const float hi = road.centre + RoadNetwork::HalfWidth;
        const auto surface = road.vertical
            ? rectPoints(lo, 0, hi, RoadNetwork::WorldHeight)
            : rectPoints(0, lo, RoadNetwork::WorldWidth, hi);
        PolygonFill::draw(target, surface, lines, Colors::Road, false);
    }
}

void City::drawMarkings(sf::RenderTarget& target, const LineRenderer& lines) const {
    const float quiet = RoadNetwork::StopLineDistance - RoadNetwork::HalfWidth;

    for (const Road& road : network_.roads()) {
        auto at = [&](float along, float across) {
            return road.vertical ? Point{road.centre + across, along}
                                 : Point{along, road.centre + across};
        };

        for (const auto& [from, to] : network_.freeIntervals(road)) {
            // Kerb lines on both sides of every free stretch of road.
            for (float side : {-RoadNetwork::HalfWidth, RoadNetwork::HalfWidth})
                lines.draw(target, at(from, side), at(to, side), Colors::RoadLine);

            // Dashed centre line, kept clear of crossings and stop lines.
            const float start = from > 0.0f ? from + quiet : from;
            const float end = to < (road.vertical ? RoadNetwork::WorldHeight
                                                  : RoadNetwork::WorldWidth)
                ? to - quiet : to;
            for (float s = start; s + 24.0f <= end; s += 48.0f)
                lines.draw(target, at(s, 0.0f), at(s + 24.0f, 0.0f), Colors::RoadLine);
        }
    }

    // Zebra crossings: stripes parallel to the traffic, spaced across the arm.
    for (const Crosswalk& cw : network_.crosswalks()) {
        const Point d = headingVector(cw.arm);
        const Point across = leftOf(d);
        const Point mid{(cw.start.x + cw.end.x) * 0.5f, (cw.start.y + cw.end.y) * 0.5f};
        const float halfLength = (RoadNetwork::CrosswalkOuter - RoadNetwork::CrosswalkInner) * 0.5f;

        for (float k = -RoadNetwork::HalfWidth + 6.0f; k <= RoadNetwork::HalfWidth - 6.0f; k += 11.0f) {
            const Point c{mid.x + across.x * k, mid.y + across.y * k};
            lines.draw(target,
                       {c.x - d.x * halfLength, c.y - d.y * halfLength},
                       {c.x + d.x * halfLength, c.y + d.y * halfLength},
                       Zebra, 4.0f);
        }
    }

    // Solid stop line across each incoming lane.
    for (const StopLine& stop : network_.stopLines())
        lines.draw(target, stop.a, stop.b, Zebra, 3.0f);
}

void City::draw(sf::RenderTarget& target,
                const LineRenderer& lines,
                bool polygonDebug) const {
    for (const Rect& block : network_.blocks())
        PolygonFill::draw(target, rectPoints(block), lines, Pavement, false);
    for (const Rect& park : parks_)
        PolygonFill::draw(target, rectPoints(park), lines, Grass, false);

    drawRoads(target, lines);
    drawMarkings(target, lines);

    const sf::Color roofs[] = {
        sf::Color(92, 104, 116), sf::Color(104, 96, 90), sf::Color(84, 98, 104)
    };
    for (std::size_t i = 0; i < buildings_.size(); ++i) {
        const Polygon& building = buildings_[i];
        PolygonFill::draw(target, building.vertices(), lines,
                          roofs[i % 3], polygonDebug);
        building.drawOutline(target, lines, Colors::Building, 2.0f);

        // Roof parapet: the same rectangle scaled down about its centre.
        Polygon parapet = building;
        const auto& v = building.vertices();
        const float w = v[1].x - v[0].x;
        const float h = v[2].y - v[1].y;
        parapet.scale((w - 14.0f) / w, (h - 14.0f) / h);
        parapet.drawOutline(target, lines, sf::Color(130, 138, 146), 1.0f);

        if (polygonDebug)
            building.drawVertices(target, Colors::Debug);
    }

    PolygonFill::draw(target, hospital_.vertices(), lines,
                      sf::Color(105, 115, 125), polygonDebug);
    hospital_.drawOutline(target, lines, Colors::Hospital, 3.0f);

    // Red cross on the hospital roof.
    const Point c = hospital_.centroid();
    lines.draw(target, {c.x - 26, c.y}, {c.x + 26, c.y}, Colors::AmbulanceAccent, 12.0f);
    lines.draw(target, {c.x, c.y - 26}, {c.x, c.y + 26}, Colors::AmbulanceAccent, 12.0f);

    if (polygonDebug)
        hospital_.drawVertices(target, Colors::Debug);
}
