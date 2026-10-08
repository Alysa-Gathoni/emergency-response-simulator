#pragma once
#include <SFML/Graphics.hpp>
#include "../polygons/Polygon.h"
#include "../lines/LineRenderer.h"

class Ambulance {
public:
    Ambulance(Point startPosition);

    void update(float deltaTime);
    void handleInput(float deltaTime);

    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool debug) const;

    Point position() const;
    float heading() const;

private:
    Polygon body_;
    Polygon cabin_;
    Point position_;
    float headingDegrees_{0.0f};

    float moveSpeed_{150.0f};
    float turnSpeed_{120.0f};

    void move(float distance);
    void turn(float degrees);
};
