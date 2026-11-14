#include "Check.h"
#include "city/RoadNetwork.h"
#include "city/TrafficLight.h"
#include "simulation/TrafficSystem.h"
#include <cmath>
#include <limits>

namespace {

// Separating axis test for two convex quads (vehicle footprints).
bool overlap(const std::vector<Point>& a, const std::vector<Point>& b) {
    for (const auto* poly : {&a, &b}) {
        for (std::size_t i = 0; i < poly->size(); ++i) {
            const Point p = (*poly)[i], q = (*poly)[(i + 1) % poly->size()];
            const Point axis{-(q.y - p.y), q.x - p.x};
            float minA = std::numeric_limits<float>::max(), maxA = -minA;
            float minB = minA, maxB = -minA;
            for (const Point& v : a) {
                const float d = v.x * axis.x + v.y * axis.y;
                minA = std::min(minA, d); maxA = std::max(maxA, d);
            }
            for (const Point& v : b) {
                const float d = v.x * axis.x + v.y * axis.y;
                minB = std::min(minB, d); maxB = std::max(maxB, d);
            }
            if (maxA < minB || maxB < minA) return false;
        }
    }
    return true;
}

RoadNetwork small() { return RoadNetwork({300, 800}, {200, 520}, 1100, 720); }

void densityScalesWithTheGrid() {
    CHECK(TrafficSystem::vehicleCountFor(TrafficDensity::Medium, 4) == 14);
    CHECK(TrafficSystem::vehicleCountFor(TrafficDensity::Low, 4) == 10);
    CHECK(TrafficSystem::vehicleCountFor(TrafficDensity::High, 4) == 20);
    CHECK(TrafficSystem::vehicleCountFor(TrafficDensity::High, 12) == 60);
}

void routesComeFromTheGrid() {
    RoadNetwork network;
    TrafficSystem traffic(network);
    const int nx = static_cast<int>(network.columns().size());
    const int ny = static_cast<int>(network.rows().size());
    const int cells = (nx - 1) * (ny - 1);
    const int expected = 2 * cells + ((nx > 2 || ny > 2) ? 2 : 0) + nx + ny + (nx - 1) + (ny - 1);
    CHECK(traffic.routeCount() == expected);
    CHECK(static_cast<int>(traffic.vehicles().size()) ==
          TrafficSystem::vehicleCountFor(TrafficDensity::Medium,
                                         static_cast<int>(network.intersections().size())));

    // Every route is closed and passes through at least one junction box.
    for (const auto& v : traffic.vehicles()) {
        const auto& path = v.route().path;
        const Point a = path.pointAtDistance(0.0f);
        const Point b = path.pointAtDistance(path.length() - 0.01f);
        CHECK(std::hypot(a.x - b.x, a.y - b.y) < 1.0f);
        CHECK(!v.route().junctions.empty());
    }
}

void signalOffsetsFollowTheGrid() {
    RoadNetwork network;
    TrafficLights lights(network);
    for (int i = 0; i < static_cast<int>(network.intersections().size()); ++i) {
        const float expected = std::fmod((network.columnOf(i) + network.rowOf(i)) * 8.0f, 24.0f);
        CHECK_NEAR(lights.controller(i).offset(), expected, 1e-4f);
    }
}

// Long run: no two vehicles may ever overlap, and nobody is stuck for ages.
void soakWithoutOverlaps(const RoadNetwork& network, TrafficDensity density,
                         unsigned seed, float seconds) {
    TrafficLights lights(network);
    TrafficSystem traffic(network, density, seed);
    const float dt = 1.0f / 60.0f;
    int overlaps = 0;
    std::vector<float> stopped(traffic.vehicles().size(), 0.0f);
    float longestStop = 0.0f;

    for (int step = 0; step < static_cast<int>(seconds / dt); ++step) {
        lights.update(dt);
        traffic.update(dt, lights, {});
        const auto& vs = traffic.vehicles();
        if (step % 6 == 0) {
            for (std::size_t i = 0; i < vs.size(); ++i)
                for (std::size_t j = i + 1; j < vs.size(); ++j)
                    if (overlap(vs[i].footprint(), vs[j].footprint())) ++overlaps;
        }
        for (std::size_t i = 0; i < vs.size(); ++i) {
            stopped[i] = vs[i].speed() < 1.0f ? stopped[i] + dt : 0.0f;
            longestStop = std::max(longestStop, stopped[i]);
        }
    }
    CHECK(overlaps == 0);
    CHECK(longestStop < 40.0f);       // a red phase plus queueing, never gridlock
}

}

void vehiclesMakeWayForTheSiren() {
    const RoadNetwork network = small();
    TrafficLights lights(network);
    TrafficSystem traffic(network, TrafficDensity::Low);

    // Put the ambulance 120 px behind the first vehicle, same direction.
    const TrafficVehicle& v = traffic.vehicles().front();
    const Point d = v.direction();
    const Point behind{v.position().x - d.x * 120.0f, v.position().y - d.y * 120.0f};
    EmergencyVehicle e{true, behind, d, 100.0f};
    for (int i = 0; i < 90; ++i) {
        // Keep the ambulance 120 px behind as the vehicle crawls on.
        const TrafficVehicle& now = traffic.vehicles().front();
        e.position = {now.position().x - now.direction().x * 120.0f,
                      now.position().y - now.direction().y * 120.0f};
        e.direction = now.direction();
        traffic.setEmergency(e);
        traffic.update(1.0f / 60.0f, lights, {});
    }
    const TrafficVehicle& after = traffic.vehicles().front();
    CHECK(after.pullingOver());
    CHECK(after.lateralOffset() > 10.0f);
    CHECK(after.speed() <= TrafficSystem::SirenCrawl + 0.5f);
    CHECK(traffic.yieldingCount() >= 1);

    // Siren off: back into the lane.
    e.siren = false;
    traffic.setEmergency(e);
    for (int i = 0; i < 90; ++i) traffic.update(1.0f / 60.0f, lights, {});
    CHECK(!traffic.vehicles().front().pullingOver());
    CHECK(traffic.vehicles().front().lateralOffset() == 0.0f);
}

void keepClearAreaStaysEmpty() {
    const RoadNetwork network = small();
    TrafficSystem traffic(network, TrafficDensity::High);
    const Point start{420.0f, 165.0f};
    traffic.keepClear(start, 140.0f);
    traffic.reset();
    for (const auto& v : traffic.vehicles())
        CHECK(std::hypot(v.position().x - start.x, v.position().y - start.y) >= 140.0f);
}

int main() {
    vehiclesMakeWayForTheSiren();
    keepClearAreaStaysEmpty();
    densityScalesWithTheGrid();
    routesComeFromTheGrid();
    signalOffsetsFollowTheGrid();
    const RoadNetwork grid2x2 = small();
    soakWithoutOverlaps(grid2x2, TrafficDensity::Medium, 7, 240.0f);
    soakWithoutOverlaps(grid2x2, TrafficDensity::High, 3, 180.0f);
    const RoadNetwork district;
    soakWithoutOverlaps(district, TrafficDensity::Medium, 5, 60.0f);
    return check::failures();
}
