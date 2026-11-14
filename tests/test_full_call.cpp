#include "Check.h"
#include "city/City.h"
#include "city/Pedestrians.h"
#include "city/RoadGraph.h"
#include "city/RoadNetwork.h"
#include "city/TrafficLight.h"
#include "simulation/Ambulance.h"
#include "simulation/CollisionSystem.h"
#include "simulation/DemoDriver.h"
#include "simulation/Mission.h"
#include "simulation/RouteGuide.h"
#include "simulation/TrafficSystem.h"
#include <cmath>

// The whole simulation loop without a window: incident -> route -> drive
// through live traffic -> treat on scene -> hospital -> next call. The
// demo driver produces the same inputs as the keyboard.
int main() {
    constexpr float Dt = 1.0f / 60.0f;
    RoadNetwork network;
    RoadGraph graph(network);
    City city(network);
    TrafficLights lights(network);
    TrafficSystem traffic(network);
    Pedestrians pedestrians(network);
    CollisionSystem collisions;
    const Point start = city.hospitalEntrance();
    Ambulance ambulance(start);
    Mission mission(graph.incidentSpots(), start);
    RouteGuide guide(graph);
    DemoDriver driver;

    bool sawOnScene = false, sawTransport = false;
    float firstRoute = 0.0f;
    guide.update(Dt, ambulance.position(), mission.target(), mission.driving());
    firstRoute = guide.remaining();

    for (int i = 0; i < static_cast<int>(240.0f / Dt) && mission.completed() < 2; ++i) {
        std::vector<Point> nearby;
        for (const auto& v : traffic.vehicles()) nearby.push_back(v.position());
        const DrivingInput input = mission.driving()
            ? driver.drive(ambulance, guide.route(), nearby, true, Dt) : DrivingInput{};

        ambulance.update(Dt, input);
        lights.update(Dt);
        pedestrians.update(Dt, lights);
        auto obstacles = pedestrians.footprints();
        obstacles.push_back(ambulance.footprint());
        traffic.update(Dt, lights, obstacles);
        collisions.update(ambulance, city, traffic, Dt);
        mission.update(Dt, ambulance.position(), ambulance.speed());
        guide.update(Dt, ambulance.position(), mission.target(), mission.driving());

        sawOnScene = sawOnScene || mission.state() == Mission::State::OnScene;
        sawTransport = sawTransport || mission.state() == Mission::State::Transport;

        // The ambulance never ends up inside a building.
        for (const Polygon& b : city.buildings())
            if (b.vertices().size() == 4)
                CHECK(Collision::sat(ambulance.footprint(), b.vertices()).depth < 4.0f);
    }

    CHECK(firstRoute > 0.0f);
    CHECK(sawOnScene);
    CHECK(sawTransport);
    CHECK(mission.completed() >= 2);
    CHECK(mission.missed() == 0);
    return check::failures();
}
