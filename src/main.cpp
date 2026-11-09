#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "core/Color.h"
#include "lines/LineRenderer.h"
#include "city/RoadNetwork.h"
#include "city/City.h"
#include "city/TrafficLight.h"
#include "city/RedLightCamera.h"
#include "city/Environment.h"
#include "simulation/Ambulance.h"
#include "simulation/ClippingDemo.h"
#include "simulation/EmergencyMarker.h"
#include "simulation/TrafficSystem.h"

namespace {

// Corners of the ambulance body (60 x 32) from its pose, so civilian
// traffic can see it in its forward corridor.
std::vector<Point> ambulanceFootprint(const Ambulance& ambulance) {
    const float a = ambulance.heading() * 3.14159265f / 180.0f;
    const Point c = ambulance.position();
    const Point d{std::cos(a), std::sin(a)};
    std::vector<Point> corners;
    for (auto [lx, ly] : {std::pair{-30.0f, -16.0f}, {30.0f, -16.0f},
                          {30.0f, 16.0f}, {-30.0f, 16.0f}})
        corners.push_back({c.x + lx * d.x - ly * d.y, c.y + lx * d.y + ly * d.x});
    return corners;
}

}

int main() {
    sf::RenderWindow window(
        sf::VideoMode(static_cast<unsigned>(RoadNetwork::WorldWidth),
                      static_cast<unsigned>(RoadNetwork::WorldHeight)),
        "Emergency Response Simulator"
    );
    window.setFramerateLimit(60);

    LineRenderer lineRenderer(LineMode::Bresenham);
    RoadNetwork network;
    City city(network);
    Environment environment;
    TrafficLights trafficLights(network);
    Ambulance ambulance({420.0f, 175.0f});
    ClippingDemo clippingDemo;
    EmergencyMarker emergency({700.0f, 545.0f});
    TrafficSystem traffic(network);
    RedLightCamera redLightCamera(network, trafficLights);

    bool graphicsDebug = false;
    bool clippingDebug = false;
    bool curveDebug = false;
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();

                if (event.key.code == sf::Keyboard::L) {
                    lineRenderer.setMode(
                        lineRenderer.getMode() == LineMode::Bresenham
                            ? LineMode::DDA
                            : LineMode::Bresenham
                    );
                }

                if (event.key.code == sf::Keyboard::G) {
                    graphicsDebug = !graphicsDebug;
                    lineRenderer.setDebug(graphicsDebug);
                }

                if (event.key.code == sf::Keyboard::C)
                    clippingDebug = !clippingDebug;

                if (event.key.code == sf::Keyboard::B)
                    curveDebug = !curveDebug;
            }
        }

        const float deltaTime = clock.restart().asSeconds();

        const Point beforeMove = ambulance.position();
        ambulance.update(deltaTime);
        redLightCamera.update(beforeMove, ambulance.position(), deltaTime);
        trafficLights.update(deltaTime);
        emergency.update(deltaTime);
        traffic.update(deltaTime, trafficLights, {ambulanceFootprint(ambulance)});

        window.clear(Colors::Background);

        city.draw(window, lineRenderer, graphicsDebug);
        environment.draw(window, graphicsDebug);
        trafficLights.draw(window, lineRenderer, graphicsDebug);
        emergency.draw(window, graphicsDebug);
        traffic.draw(window, lineRenderer, curveDebug, graphicsDebug);
        ambulance.draw(window, lineRenderer, graphicsDebug);
        redLightCamera.draw(window, lineRenderer);
        clippingDemo.draw(window, lineRenderer, clippingDebug);

        window.display();
    }

    return 0;
}
