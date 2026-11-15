#pragma once
#include "render/IRenderer.h"
#include <string>
#include <vector>
#include "RoadNetwork.h"
#include "../lines/LineRenderer.h"
#include "../polygons/Polygon.h"

class City {
public:
    explicit City(const RoadNetwork& network);

    void draw(IRenderer& target,
              const LineRenderer& lines,
              bool polygonDebug) const;

    const std::vector<Polygon>& buildings() const;
    const Polygon& hospital() const;
    const std::vector<Rect>& parks() const;
    const RoadNetwork& network() const;

    // Where the ambulance waits: the kerbside lane in front of the hospital,
    // facing east.
    Point hospitalEntrance() const;

private:
    const RoadNetwork& network_;
    std::vector<Polygon> buildings_;
    Polygon hospital_;
    std::vector<Rect> parks_;

    void drawRoads(IRenderer& target, const LineRenderer& lines) const;
    void drawMarkings(IRenderer& target, const LineRenderer& lines) const;
    void drawLaneArrows(IRenderer& target, const LineRenderer& lines) const;
    void drawRoadNames(IRenderer& target) const;

public:
    // Arrow painted in each lane before the stop line: a concave 7-vertex
    // outline pointing along +x, placed with T(position) * R(heading).
    static std::vector<Point> laneArrow();
    // Where a road name is painted: one label in every other road section,
    // near the kerb, rotated -90 degrees on north-south roads.
    struct Label {
        std::string text;
        Point centre;
        float degrees;
    };
    std::vector<Label> roadLabels() const;
};
