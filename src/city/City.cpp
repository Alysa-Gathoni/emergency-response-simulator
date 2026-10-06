#include "city/City.h"
#include <array>

City::City(LineAlgorithm algorithm) : algorithm_(algorithm) {}
void City::setAlgorithm(LineAlgorithm algorithm) { algorithm_ = algorithm; }
LineAlgorithm City::algorithm() const { return algorithm_; }
void City::setDebug(bool enabled) { debug_ = enabled; }
bool City::debug() const { return debug_; }

void City::rasterLine(sf::RenderTarget& target, Point a, Point b, sf::Color color, int thickness) const {
    const auto points = algorithm_ == LineAlgorithm::DDA
        ? LineAlgorithms::dda(a.x, a.y, b.x, b.y)
        : LineAlgorithms::bresenham(a.x, a.y, b.x, b.y);
    sf::VertexArray pixels(sf::Points, points.size());
    for (std::size_t i = 0; i < points.size(); ++i) {
        pixels[i].position = sf::Vector2f(static_cast<float>(points[i].x), static_cast<float>(points[i].y));
        pixels[i].color = color;
    }
    for (int ox = -(thickness/2); ox <= thickness/2; ++ox) {
        for (int oy = -(thickness/2); oy <= thickness/2; ++oy) {
            sf::Transform transform;
            transform.translate(static_cast<float>(ox), static_cast<float>(oy));
            sf::RenderStates states(transform);
            target.draw(pixels, states);
        }
    }
    if (debug_) {
        for (std::size_t i = 0; i < points.size(); i += 12) {
            sf::RectangleShape marker({3.f, 3.f});
            marker.setPosition(static_cast<float>(points[i].x - 1), static_cast<float>(points[i].y - 1));
            marker.setFillColor(sf::Color(255, 80, 80));
            target.draw(marker);
        }
    }
}

void City::drawRoads(sf::RenderTarget& target) const {
    // Two major roads form the checkpoint's compact city block.
    const sf::Color edge(225, 225, 225), lane(245, 200, 80);
    rasterLine(target, {0, 250}, {960, 250}, edge, 3);
    rasterLine(target, {0, 390}, {960, 390}, edge, 3);
    rasterLine(target, {390, 0}, {390, 640}, edge, 3);
    rasterLine(target, {540, 0}, {540, 640}, edge, 3);
    // Dashed centre lines are individually rasterized by our own algorithm.
    for (int x = 0; x < 960; x += 55) rasterLine(target, {x, 320}, {x + 30, 320}, lane, 2);
    for (int y = 0; y < 640; y += 55) rasterLine(target, {465, y}, {465, y + 30}, lane, 2);
    // Crosswalks around the central intersection.
    for (int x = 405; x <= 525; x += 20) {
        rasterLine(target, {x, 260}, {x, 285}, sf::Color::White, 2);
        rasterLine(target, {x, 355}, {x, 380}, sf::Color::White, 2);
    }
}

void City::drawBuildings(sf::RenderTarget& target) const {
    const std::array<std::array<Point,4>, 6> blocks{{
        {{{60,55},{300,55},{300,205},{60,205}}},
        {{{610,55},{875,55},{875,205},{610,205}}},
        {{{55,440},{300,440},{300,585},{55,585}}},
        {{{610,440},{875,440},{875,585},{610,585}}},
        {{{325,75},{365,75},{365,205},{325,205}}},
        {{{565,440},{600,440},{600,565},{565,565}}}
    }};
    for (const auto& p : blocks) {
        rasterLine(target,p[0],p[1],sf::Color(160,190,205),3);
        rasterLine(target,p[1],p[2],sf::Color(160,190,205),3);
        rasterLine(target,p[2],p[3],sf::Color(160,190,205),3);
        rasterLine(target,p[3],p[0],sf::Color(160,190,205),3);
    }
    // Hospital 'H' made entirely from rasterized lines.
    rasterLine(target,{135,95},{135,165},sf::Color(235,235,235),5);
    rasterLine(target,{195,95},{195,165},sf::Color(235,235,235),5);
    rasterLine(target,{135,130},{195,130},sf::Color(235,235,235),5);
}

void City::draw(sf::RenderTarget& target) const {
    drawRoads(target);
    drawBuildings(target);
}
