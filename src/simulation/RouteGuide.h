#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "../city/RoadNetwork.h"
#include "../lines/LineRenderer.h"

// Shortest route along the roads from the ambulance to the incident.
//
// The road network becomes a small weighted graph:
//   nodes = intersection centres + the points where the ambulance and the
//           incident join their nearest road centre line
//   edges = consecutive nodes on the same road, weight = distance
// Dijkstra's algorithm finds the shortest node sequence. The route is
// re-planned every frame, so it follows the ambulance if the driver takes
// a different turn.
//
// It is drawn as dashes flowing towards the incident, plus an arrow next
// to the ambulance pointing along the first leg.
class RouteGuide {
public:
    // Polyline from `from` to `to` (both included) following the roads.
    static std::vector<Point> plan(const RoadNetwork& network, Point from, Point to);
    static float length(const std::vector<Point>& route);

    explicit RouteGuide(const RoadNetwork& network);

    void update(float deltaTime, Point ambulance, Point incident, bool active);
    void draw(sf::RenderTarget& target, const LineRenderer& lines,
              sf::Color colour, bool debug) const;

    const std::vector<Point>& route() const { return route_; }
    float remaining() const { return length(route_); }

private:
    const RoadNetwork& network_;
    std::vector<Point> route_;
    float phase_{0.0f};
    bool active_{false};
};
