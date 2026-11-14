#pragma once
#include "render/IRenderer.h"
#include <vector>
#include "../city/RoadGraph.h"
#include "../lines/LineRenderer.h"

// How jammed each road section (RoadGraph edge) is, from the live traffic.
//
// Every vehicle is assigned to the edge whose centre line it is on. A
// queued vehicle (below SlowSpeed, e.g. waiting at a red light) costs about
// QueueDelay seconds, a moving one MovingDelay; turned into extra pixels at
// the par speed and spread over the edge:
//
//   raw(e)    = 1 + (queued * QueueDelay + moving * MovingDelay) * ParSpeed / length(e)
//   factor(e) = factor(e) + (min(raw, MaxFactor) - factor(e)) * (1 - e^(-dt / Smoothing))
//
// factor >= 1 multiplies the edge length in A*, so the route avoids queues
// (the straight-line heuristic stays admissible because weights only grow).
// Smoothing keeps the route from flickering when one car stops for a moment.
// Vehicles pulling over for the siren are moving out of the way and are
// counted as moving.
class Congestion {
public:
    static constexpr float SlowSpeed = 15.0f;     // px/s
    static constexpr float QueueDelay = 3.0f;     // s per queued vehicle
    static constexpr float MovingDelay = 0.3f;    // s per moving vehicle
    static constexpr float ParSpeed = 90.0f;      // px/s, as in the scoring
    static constexpr float MaxFactor = 4.0f;
    static constexpr float Smoothing = 1.5f;      // s

    struct Sample {
        Point position;
        float speed;
        bool yielding;     // pulling over for the siren
    };

    explicit Congestion(const RoadGraph& graph);

    void update(float deltaTime, const std::vector<Sample>& vehicles);
    void reset();

    const std::vector<float>& factors() const { return factors_; }
    float level(int edge) const;     // 0 (free) .. 1 (MaxFactor)

    // V overlay: every road section coloured green -> amber -> red.
    void draw(IRenderer& target, const LineRenderer& lines) const;

private:
    const RoadGraph& graph_;
    std::vector<float> factors_;
};
