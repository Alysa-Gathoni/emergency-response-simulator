#pragma once
#include <SFML/Graphics.hpp>
#include "lines/LineAlgorithms.h"

class City {
public:
    explicit City(LineAlgorithm algorithm = LineAlgorithm::Bresenham);
    void setAlgorithm(LineAlgorithm algorithm);
    LineAlgorithm algorithm() const;
    void setDebug(bool enabled);
    bool debug() const;
    void draw(sf::RenderTarget& target) const;
private:
    LineAlgorithm algorithm_;
    bool debug_{false};
    void rasterLine(sf::RenderTarget& target, Point a, Point b, sf::Color color, int thickness = 1) const;
    void drawRoads(sf::RenderTarget& target) const;
    void drawBuildings(sf::RenderTarget& target) const;
};
