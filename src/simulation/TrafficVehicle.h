#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "VehicleType.h"
#include "../curves/Spline.h"
#include "../lines/LineRenderer.h"

// A closed route plus the places along it where traffic may have to stop.
struct TrafficRoute {
    struct StopPoint {
        float distance;   // arc length at which the path crosses the stop line
        int stopLine;     // index into RoadNetwork::stopLines()
    };

    SplinePath path;
    std::vector<StopPoint> stops;
    std::vector<float> stages;   // arc lengths of matatu stages on this route
};

// Why a vehicle is not driving at its cruise speed (shown in the debug view).
enum class SpeedLimiter { Free, Corner, Signal, Leader, Stage };

class TrafficVehicle {
public:
    // cruiseSpeed is in pixels per second along the path; paletteIndex
    // picks the body colour so the traffic is not all one colour.
    TrafficVehicle(std::shared_ptr<const TrafficRoute> route,
                   VehicleType type, float cruiseSpeed,
                   float startDistance, int paletteIndex);

    // Planning and integration are separate so every vehicle plans from the
    // same snapshot of the traffic before anyone moves.
    void setTarget(float targetSpeed, SpeedLimiter reason, Point blockingPoint);
    void integrate(float deltaTime);

    const TrafficRoute& route() const;
    VehicleType type() const;
    const VehicleSpec& spec() const;
    int servedStage() const;          // stage just served (-1 if none)
    float distance() const;           // arc length of the centre along the route
    float speed() const;
    float cruiseSpeed() const;
    float comfortableBraking() const; // b in v^2 = 2 b d
    float halfLength() const;
    float halfWidth() const;

    Point position() const;
    Point direction() const;          // unit vector of travel
    std::vector<Point> footprint() const;

    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool curveDebug,
              bool trafficDebug) const;

private:
    std::shared_ptr<const TrafficRoute> route_;
    const VehicleSpec* spec_;
    float cruiseSpeed_;
    float distance_;
    int paletteIndex_;
    float speed_{0.0f};

    float dwell_{0.0f};      // remaining stop time at a matatu stage
    int servedStage_{-1};

    float targetSpeed_{0.0f};
    SpeedLimiter limiter_{SpeedLimiter::Free};
    Point blockingPoint_{};

    void updateStage();
    void drawBody(sf::RenderTarget& target, const LineRenderer& lines) const;
};
