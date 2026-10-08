#pragma once
#include <SFML/Graphics.hpp>
#include "../lines/LineRenderer.h"
#include "../polygons/Polygon.h"

class City {
public:
    City();
    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool polygonDebug) const;

private:
    std::vector<Polygon> buildings_;
    void drawRoads(sf::RenderTarget& target, const LineRenderer& lines) const;
};
