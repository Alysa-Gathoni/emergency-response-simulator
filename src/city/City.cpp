#include "City.h"
#include <algorithm>
#include <cmath>
#include <random>
#include "../core/Color.h"
#include "../rasterization/PolygonFill.h"

namespace {

std::vector<Point> rectPoints(float l, float t, float r, float b) {
    return {{l, t}, {r, t}, {r, b}, {l, b}};
}

std::vector<Point> rectPoints(const Rect& r) {
    return rectPoints(r.left, r.top, r.right, r.bottom);
}

const Rgba Pavement(58, 64, 72);
const Rgba Grass(46, 84, 54);
const Rgba Zebra(225, 225, 225);

}

City::City(const RoadNetwork& network) : network_(network) {
    // The layout is generated from the road grid, so a new map needs no
    // hand-placed coordinates. Every block (land between roads) gets a 14 px
    // pavement; inside it:
    //   - the block south-east of the first junction holds the hospital,
    //   - three blocks are parks,
    //   - every other block is split into a grid of building lots whose
    //     size follows the block (about 150 x 140 px each), with the split
    //     lines jittered by a fixed-seed random generator so streets do not
    //     look stamped out.
    const auto xs = network.columns();
    const auto ys = network.rows();
    const float edge = RoadNetwork::HalfWidth;
    const float pavement = 14.0f;
    std::mt19937 rng(11);
    std::uniform_real_distribution<float> jitter(-0.18f, 0.18f);

    auto blockAt = [&](float x, float y) {
        for (const Rect& b : network.blocks())
            if (x > b.left && x < b.right && y > b.top && y < b.bottom) return b;
        return Rect{0, 0, 0, 0};
    };
    const Rect hospitalBlock = blockAt(xs[0] + edge + 5, ys[0] + edge + 5);
    const Rect parkBlocks[] = {
        blockAt(xs.back() + edge + 5, ys[0] - edge - 5),                      // north-east corner
        blockAt(xs[0] - edge - 5, ys.back() + edge + 5),                      // south-west corner
        blockAt(xs[xs.size() / 2] + edge + 5, ys[ys.size() / 2] + edge + 5),  // "central park"
    };
    auto same = [](const Rect& a, const Rect& b) {
        return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
    };

    for (const Rect& block : network.blocks()) {
        const Rect inner{block.left + pavement, block.top + pavement,
                         block.right - pavement, block.bottom - pavement};
        const float w = inner.right - inner.left;
        const float h = inner.bottom - inner.top;
        if (w < 40.0f || h < 40.0f) continue;

        if (same(block, hospitalBlock)) {
            // Hospital fills the block except a strip for one more building.
            const float split = inner.left + w * 0.68f;
            hospital_ = Polygon(rectPoints(inner.left, inner.top, split, inner.bottom));
            buildings_.emplace_back(rectPoints(split + pavement, inner.top, inner.right, inner.bottom));
            continue;
        }
        bool park = false;
        for (const Rect& p : parkBlocks) park = park || same(block, p);
        if (park) {
            parks_.push_back(inner);
            continue;
        }

        const int cols = std::clamp(static_cast<int>(std::lround(w / 150.0f)), 1, 4);
        const int rowsHere = std::clamp(static_cast<int>(std::lround(h / 140.0f)), 1, 3);
        std::vector<float> cx = {inner.left}, cy = {inner.top};
        for (int c = 1; c < cols; ++c) cx.push_back(inner.left + w * (c + jitter(rng) * 0.5f) / cols);
        for (int r = 1; r < rowsHere; ++r) cy.push_back(inner.top + h * (r + jitter(rng) * 0.5f) / rowsHere);
        cx.push_back(inner.right);
        cy.push_back(inner.bottom);

        for (int c = 0; c < cols; ++c) {
            for (int r = 0; r < rowsHere; ++r) {
                const float l = cx[c] + (c > 0 ? pavement * 0.5f : 0.0f);
                const float t = cy[r] + (r > 0 ? pavement * 0.5f : 0.0f);
                const float rr = cx[c + 1] - (c + 1 < cols ? pavement * 0.5f : 0.0f);
                const float b = cy[r + 1] - (r + 1 < rowsHere ? pavement * 0.5f : 0.0f);
                if (rr - l > 30.0f && b - t > 30.0f)
                    buildings_.emplace_back(rectPoints(l, t, rr, b));
            }
        }
    }
}

const RoadNetwork& City::network() const { return network_; }

Point City::hospitalEntrance() const {
    // Street north of the hospital, eastbound lane (left-hand traffic).
    const Point c = hospital_.centroid();
    const float street = network_.rows().front();
    return {c.x, street - RoadNetwork::LaneOffset};
}

const std::vector<Polygon>& City::buildings() const { return buildings_; }
const Polygon& City::hospital() const { return hospital_; }
const std::vector<Rect>& City::parks() const { return parks_; }

void City::drawRoads(IRenderer& target, const LineRenderer& lines) const {
    // Road surfaces are rectangles filled with our scan-line algorithm.
    for (const Road& road : network_.roads()) {
        const float lo = road.centre - RoadNetwork::HalfWidth;
        const float hi = road.centre + RoadNetwork::HalfWidth;
        const auto surface = road.vertical
            ? rectPoints(lo, 0, hi, network_.height())
            : rectPoints(0, lo, network_.width(), hi);
        PolygonFill::draw(target, surface, lines, Colors::Road, false);
    }
}

void City::drawMarkings(IRenderer& target, const LineRenderer& lines) const {
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
            const float end = to < (road.vertical ? network_.height()
                                                  : network_.width())
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

void City::draw(IRenderer& target,
                const LineRenderer& lines,
                bool polygonDebug) const {
    for (const Rect& block : network_.blocks())
        PolygonFill::draw(target, rectPoints(block), lines, Pavement, false);
    for (const Rect& park : parks_)
        PolygonFill::draw(target, rectPoints(park), lines, Grass, false);

    drawRoads(target, lines);
    drawMarkings(target, lines);

    const Rgba roofs[] = {
        Rgba(92, 104, 116), Rgba(104, 96, 90), Rgba(84, 98, 104)
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
        parapet.drawOutline(target, lines, Rgba(130, 138, 146), 1.0f);

        if (polygonDebug)
            building.drawVertices(target, Colors::Debug);
    }

    PolygonFill::draw(target, hospital_.vertices(), lines,
                      Rgba(105, 115, 125), polygonDebug);
    hospital_.drawOutline(target, lines, Colors::Hospital, 3.0f);

    // Red cross on the hospital roof.
    const Point c = hospital_.centroid();
    lines.draw(target, {c.x - 26, c.y}, {c.x + 26, c.y}, Colors::AmbulanceAccent, 12.0f);
    lines.draw(target, {c.x, c.y - 26}, {c.x, c.y + 26}, Colors::AmbulanceAccent, 12.0f);

    if (polygonDebug)
        hospital_.drawVertices(target, Colors::Debug);
}
