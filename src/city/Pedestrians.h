#pragma once
#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include "RoadNetwork.h"
#include "TrafficLight.h"
#include "../lines/LineRenderer.h"

// People crossing at the zebra crossings.
//
// A crossing on the north arm of a junction is driven over by north-south
// traffic, so pedestrians may walk there while the EAST-WEST group is
// green (north-south is then red). They only start if enough green time is
// left to finish; anyone already on the road always completes the crossing.
class Pedestrians {
public:
    Pedestrians(const RoadNetwork& network, unsigned seed = 11);

    void update(float deltaTime, const TrafficLights& lights);
    void draw(sf::RenderTarget& target, const LineRenderer& lines,
              const TrafficLights& lights, bool debug) const;

    // Small boxes around people on the road, so traffic gives way to them.
    std::vector<std::vector<Point>> footprints() const;

    bool walkAllowed(const Crosswalk& crosswalk, const TrafficLights& lights) const;

private:
    struct Walker {
        int crosswalk;
        bool forward;       // start -> end, otherwise end -> start
        bool crossing;      // false while waiting at the kerb
        float progress;     // 0..1 along the crossing
        float stepPhase;    // drives the leg swing
        float speed;        // px/s
        int palette;
    };

    const RoadNetwork& network_;
    std::vector<Walker> walkers_;
    std::vector<float> spawnTimers_;   // one per crosswalk end
    std::mt19937 rng_;

    Point positionOf(const Walker& w) const;
    Point directionOf(const Walker& w) const;
};
