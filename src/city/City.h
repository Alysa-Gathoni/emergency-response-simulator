#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "RoadNetwork.h"
#include "../lines/LineRenderer.h"
#include "../polygons/Polygon.h"

class City {
public:
    explicit City(const RoadNetwork& network);

    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool polygonDebug) const;

    const std::vector<Polygon>& buildings() const;
    const Polygon& hospital() const;
    const std::vector<Rect>& parks() const;

private:
    const RoadNetwork& network_;
    std::vector<Polygon> buildings_;
    Polygon hospital_;
    std::vector<Rect> parks_;

    void drawRoads(sf::RenderTarget& target, const LineRenderer& lines) const;
    void drawMarkings(sf::RenderTarget& target, const LineRenderer& lines) const;
};
