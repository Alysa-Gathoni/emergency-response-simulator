#pragma once
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>
#include "RoadNetwork.h"
#include "SignalController.h"
#include "../lines/LineRenderer.h"

// One SignalController per intersection plus the drawing of the signal
// heads. Every approach gets its own head on the left kerb (left-hand
// traffic), showing the state of that approach's signal group.
class TrafficLights {
public:
    explicit TrafficLights(const RoadNetwork& network);

    void update(float deltaTime);

    // All intersections back to the start of their cycles (restart key).
    void reset();

    Signal signalAt(int intersection, SignalGroup group) const;
    Signal signalFor(const StopLine& stopLine) const;
    const SignalController& controller(int intersection) const;

    // Centre and colour of every lit bulb (used for the night glow).
    std::vector<std::pair<Point, sf::Color>> litBulbs() const;

    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool debug) const;

private:
    const RoadNetwork& network_;
    std::vector<SignalController> controllers_;
};
