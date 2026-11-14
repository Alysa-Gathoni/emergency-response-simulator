#pragma once
#include "render/IRenderer.h"
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
//   broad phase   AABB of the ambulance vs the AABB of every obstacle:
//                 rejects almost everything with four comparisons
//   narrow phase  SAT on the real shapes: the rotated ambulance body vs
//                 rotated vehicles, rectangles and the ear-clipped
//                 triangles of concave (L-shaped) buildings
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

    // Per-frame counts for the debug view: obstacles in total, passed the
    // broad phase, actually touching after SAT.
    int obstacleCount() const { return static_cast<int>(staticPieces_.size() + vehicles_.size()); }
    int broadPhaseHits() const { return broadHits_; }
    int narrowPhaseHits() const { return static_cast<int>(hits_.size()); }

    // G debug view: broad-phase candidates (boxes) in yellow, the shapes
    // SAT tested in orange, hits in red, ambulance body in green.
    void draw(IRenderer& target, const LineRenderer& lines, bool debug) const;

private:
    // One convex piece of an obstacle, with its box for the broad phase.
    struct Piece {
        std::vector<Point> shape;
        AABB box;
    };

    int collisions_{0};
    bool wasInContact_{false};
    float cooldown_{0.0f};

    // Buildings never move: their convex pieces are built once.
    std::vector<Piece> staticPieces_;
    const City* builtFor_{nullptr};

    std::vector<Piece> vehicles_;      // rebuilt every frame
    std::vector<Point> ambulanceShape_;
    AABB ambulanceBox_{};
    std::vector<AABB> candidates_;
    std::vector<std::vector<Point>> tested_;
    std::vector<std::vector<Point>> hits_;
    int broadHits_{0};

    void buildStatic(const City& city);
};
