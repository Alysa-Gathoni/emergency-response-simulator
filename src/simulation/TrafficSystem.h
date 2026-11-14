#pragma once
#include "render/IRenderer.h"
#include <memory>
#include <vector>
#include "TrafficVehicle.h"
#include "../city/RoadNetwork.h"
#include "../city/TrafficLight.h"

// How busy the roads are; used for the difficulty levels.
enum class TrafficDensity { Low, Medium, High };

class TrafficSystem {
public:
    // Routes are generated from the road grid (any number of roads), then
    // vehicles are spawned on random routes with the type mix from
    // VehicleType.h; the seed makes every run start the same way.
    explicit TrafficSystem(const RoadNetwork& network,
                           TrafficDensity density = TrafficDensity::Medium,
                           unsigned seed = 7);

    // Vehicles per intersection: low 2.5, medium 3.5, high 5.
    static int vehicleCountFor(TrafficDensity density, int intersections);

    // Changes the density and respawns the traffic.
    void setDensity(TrafficDensity density);
    TrafficDensity density() const;
    int routeCount() const;

    // `obstacles` are footprints of other road users the traffic must not
    // drive into (the ambulance). Civilian vehicles stop for red lights,
    // queue behind each other and keep a safe gap.
    void update(float deltaTime,
                const TrafficLights& lights,
                const std::vector<std::vector<Point>>& obstacles);

    // Puts every vehicle back where it started (same seed, so the same
    // positions, types and speeds as at launch). Used by the restart key.
    void reset();

    void draw(IRenderer& target,
              const LineRenderer& lines,
              bool curveDebug,
              bool trafficDebug) const;

    const std::vector<TrafficVehicle>& vehicles() const;
    const std::vector<Point>& stages() const;

private:
    const RoadNetwork& network_;
    std::vector<std::shared_ptr<const TrafficRoute>> routes_;
    std::vector<TrafficVehicle> vehicles_;
    std::vector<Point> stages_;   // matatu stages (lane centre at the kerb)
    TrafficDensity density_;
    unsigned seed_;

    void spawn(int count, unsigned seed);
    void buildRoutes();
    void placeStages();
    void drawStages(IRenderer& target, const LineRenderer& lines) const;

    std::shared_ptr<const TrafficRoute> makeRoute(const std::vector<Point>& waypoints) const;
    void plan(const TrafficLights& lights,
              const std::vector<std::vector<Point>>& obstacles);
};
