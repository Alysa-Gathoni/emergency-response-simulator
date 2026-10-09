#include "City.h"
#include "../core/Color.h"
#include "../rasterization/PolygonFill.h"
#include "../rasterization/Clipping.h"

City::City() {
    buildings_.emplace_back(std::vector<Point>{{55,55},{250,55},{250,190},{55,190}});
    buildings_.emplace_back(std::vector<Point>{{550,55},{790,55},{790,190},{550,190}});
    buildings_.emplace_back(std::vector<Point>{{55,420},{250,420},{250,650},{55,650}});
    buildings_.emplace_back(std::vector<Point>{{550,420},{790,420},{790,650},{550,650}});
}

void City::drawRoads(sf::RenderTarget& target, const LineRenderer& lines) const {
    lines.draw(target, {0,250}, {900,250}, Colors::RoadLine);
    lines.draw(target, {0,370}, {900,370}, Colors::RoadLine);
    lines.draw(target, {330,0}, {330,700}, Colors::RoadLine);
    lines.draw(target, {470,0}, {470,700}, Colors::RoadLine);

    for (int x = 0; x < 900; x += 55)
        lines.draw(target, {(float)x,310}, {(float)(x+28),310}, Colors::RoadLine);

    for (int y = 0; y < 700; y += 55)
        lines.draw(target, {400,(float)y}, {400,(float)(y+28)}, Colors::RoadLine);

    for (int x = 345; x <= 445; x += 20)
        lines.draw(target, {(float)x,260}, {(float)x,290}, sf::Color(230,230,230), 3.0f);
}

void City::draw(sf::RenderTarget& target,
                const LineRenderer& lines,
                bool polygonDebug) const {
    sf::RectangleShape horizontalRoad({900.0f,120.0f});
    horizontalRoad.setPosition(0,250);
    horizontalRoad.setFillColor(Colors::Road);
    target.draw(horizontalRoad);

    sf::RectangleShape verticalRoad({140.0f,700.0f});
    verticalRoad.setPosition(330,0);
    verticalRoad.setFillColor(Colors::Road);
    target.draw(verticalRoad);

    drawRoads(target, lines);

    const sf::Color buildingFill(92, 104, 116);
    for (const Polygon& building : buildings_) {
        PolygonFill::draw(target, building.vertices(), lines,
                          buildingFill, polygonDebug);
        building.drawOutline(target, lines, Colors::Building, 2.0f);
        if (polygonDebug)
            building.drawVertices(target, Colors::Debug);
    }

    Polygon hospital({{75,75},{230,75},{230,170},{75,170}});
    PolygonFill::draw(target, hospital.vertices(), lines,
                      sf::Color(105,115,125), polygonDebug);
    hospital.drawOutline(target, lines, Colors::Hospital, 3.0f);

    lines.draw(target, {135,105}, {170,105}, Colors::AmbulanceAccent, 5.0f);
    lines.draw(target, {152,88}, {152,123}, Colors::AmbulanceAccent, 5.0f);
}
