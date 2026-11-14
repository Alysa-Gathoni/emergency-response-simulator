#include "Congestion.h"
#include "../city/RoadNetwork.h"
#include <algorithm>
#include <cmath>

Congestion::Congestion(const RoadGraph& graph)
    : graph_(graph), factors_(graph.edges().size(), 1.0f) {}

void Congestion::reset() {
    std::fill(factors_.begin(), factors_.end(), 1.0f);
}

void Congestion::update(float deltaTime, const std::vector<Sample>& vehicles) {
    const auto& edges = graph_.edges();
    std::vector<int> queued(edges.size(), 0), moving(edges.size(), 0);
    for (const Sample& v : vehicles) {
        const RoadGraph::EdgePoint hit = graph_.nearestEdgePoint(v.position);
        if (hit.edge < 0 || hit.distance > RoadNetwork::HalfWidth + 5.0f) continue;
        if (v.speed < SlowSpeed && !v.yielding) ++queued[static_cast<std::size_t>(hit.edge)];
        else ++moving[static_cast<std::size_t>(hit.edge)];
    }

    const float alpha = 1.0f - std::exp(-deltaTime / Smoothing);
    for (std::size_t e = 0; e < edges.size(); ++e) {
        const float extra = (queued[e] * QueueDelay + moving[e] * MovingDelay) * ParSpeed;
        const float raw = std::min(MaxFactor, 1.0f + extra / std::max(1.0f, edges[e].length));
        factors_[e] += (raw - factors_[e]) * alpha;
    }
}

float Congestion::level(int edge) const {
    if (edge < 0 || edge >= static_cast<int>(factors_.size())) return 0.0f;
    return std::clamp((factors_[static_cast<std::size_t>(edge)] - 1.0f) / (MaxFactor - 1.0f), 0.0f, 1.0f);
}

void Congestion::draw(IRenderer& target, const LineRenderer& lines) const {
    // Colour ramp green (free) -> amber -> red (jammed), linear in two halves.
    auto ramp = [](float t) {
        const Rgba green(80, 220, 120), amber(245, 180, 50), red(235, 60, 60);
        const Rgba& a = t < 0.5f ? green : amber;
        const Rgba& b = t < 0.5f ? amber : red;
        const float k = t < 0.5f ? t * 2.0f : (t - 0.5f) * 2.0f;
        auto mix = [k](int x, int y) { return static_cast<int>(std::lround(x + (y - x) * k)); };
        return Rgba(mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b), 200);
    };
    const auto& nodes = graph_.nodes();
    const auto& edges = graph_.edges();
    for (std::size_t e = 0; e < edges.size(); ++e) {
        const Point a = nodes[static_cast<std::size_t>(edges[e].from)].position;
        const Point b = nodes[static_cast<std::size_t>(edges[e].to)].position;
        lines.draw(target, a, b, ramp(level(static_cast<int>(e))), 5.0f);
    }
}
