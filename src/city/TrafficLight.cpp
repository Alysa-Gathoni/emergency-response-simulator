#include "TrafficLight.h"
#include "../shapes/CircleEllipse.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"

TrafficLight::TrafficLight(Point position) : position_(position) {}

void TrafficLight::advanceSignal() {
    if (signal_ == TrafficSignal::Red)
        signal_ = TrafficSignal::Green;
    else if (signal_ == TrafficSignal::Green)
        signal_ = TrafficSignal::Yellow;
    else
        signal_ = TrafficSignal::Red;

    elapsed_ = 0.0f;
}

void TrafficLight::update(float deltaTime) {
    elapsed_ += deltaTime;

    float duration = 5.0f;
    if (signal_ == TrafficSignal::Green) duration = 7.0f;
    if (signal_ == TrafficSignal::Yellow) duration = 2.0f;

    if (elapsed_ >= duration)
        advanceSignal();
}

void TrafficLight::draw(sf::RenderTarget& target,
                        const LineRenderer& lines,
                        bool debug) const {
    Polygon housing({
        {position_.x - 13, position_.y - 34},
        {position_.x + 13, position_.y - 34},
        {position_.x + 13, position_.y + 34},
        {position_.x - 13, position_.y + 34}
    });

    PolygonFill::draw(target, housing.vertices(), lines,
                      sf::Color(30,30,30), false);
    housing.drawOutline(target, lines, sf::Color(190,190,190), 2.0f);

    const Point red{position_.x, position_.y - 21};
    const Point yellow{position_.x, position_.y};
    const Point green{position_.x, position_.y + 21};

    const sf::Color off(75,75,75);

    ShapeAlgorithms::drawPoints(
        target, ShapeAlgorithms::midpointCircle(red, 7),
        signal_ == TrafficSignal::Red ? sf::Color(235,70,70) : off, 2.0f);

    ShapeAlgorithms::drawPoints(
        target, ShapeAlgorithms::midpointCircle(yellow, 7),
        signal_ == TrafficSignal::Yellow ? sf::Color(240,205,70) : off, 2.0f);

    ShapeAlgorithms::drawPoints(
        target, ShapeAlgorithms::midpointCircle(green, 7),
        signal_ == TrafficSignal::Green ? sf::Color(80,215,105) : off, 2.0f);

    if (debug) {
        for (Point p : {red, yellow, green}) {
            ShapeAlgorithms::drawPoints(
                target, ShapeAlgorithms::midpointCircle(p, 10),
                sf::Color(80,220,170), 1.0f);
        }
    }
}

TrafficSignal TrafficLight::signal() const {
    return signal_;
}
