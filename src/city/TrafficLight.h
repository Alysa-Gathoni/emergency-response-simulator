#pragma once
#include <SFML/Graphics.hpp>
#include "../core/Point.h"
#include "../lines/LineRenderer.h"

enum class TrafficSignal { Red, Green, Yellow };

class TrafficLight {
public:
    explicit TrafficLight(Point position);

    void update(float deltaTime);
    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool debug) const;

    TrafficSignal signal() const;

private:
    Point position_;
    TrafficSignal signal_{TrafficSignal::Red};
    float elapsed_{0.0f};

    void advanceSignal();
};
