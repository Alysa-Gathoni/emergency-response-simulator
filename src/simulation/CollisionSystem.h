#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Collision.h"
#include "../lines/LineRenderer.h"

class Ambulance;
class City;
class TrafficSystem;

// Checks the ambulance against buildings, the hospital, civilian vehicles
// and the edge of the map every frame, pushes it out of anything it hits
// and counts collisions for the assessment.
//
// A collision is counted once when contact begins (not every frame of a
// long scrape), and a short cooldown stops one bump being counted twice.
class CollisionSystem {
public:
    void update(Ambulance& ambulance,
                const City& city,
                const TrafficSystem& traffic,
                float deltaTime);

    int collisions() const;
    bool inContact() const;
    void reset();

    // G debug view: boxes that were tested this frame, hits in red.
    void draw(sf::RenderTarget& target, const LineRenderer& lines, bool debug) const;

private:
    int collisions_{0};
    bool wasInContact_{false};
    float cooldown_{0.0f};

    AABB ambulanceBox_{};
    std::vector<AABB> nearby_;
    std::vector<AABB> hits_;
};
