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
#include "city/Pedestrians.h"
#include "city/DayNight.h"
#include "simulation/Ambulance.h"
#include "simulation/ClippingDemo.h"
#include "simulation/CollisionSystem.h"
#include "simulation/Mission.h"
#include "simulation/RouteGuide.h"
#include "simulation/TrafficSystem.h"

int main() {
    sf::RenderWindow window(
        sf::VideoMode(static_cast<unsigned>(RoadNetwork::WorldWidth),
                      static_cast<unsigned>(RoadNetwork::WorldHeight)),
        "Emergency Response Simulator"
    );
    window.setFramerateLimit(60);

    const Point startPosition{420.0f, 175.0f};   // outside the hospital

    LineRenderer lineRenderer(LineMode::Bresenham);
    RoadNetwork network;
    City city(network);
    Environment environment;
    Pedestrians pedestrians(network);
    DayNight dayNight;
    TrafficLights trafficLights(network);
    Ambulance ambulance(startPosition);
    ClippingDemo clippingDemo;
    Mission mission;
    RouteGuide routeGuide(network);
    TrafficSystem traffic(network);
    RedLightCamera redLightCamera(network, trafficLights);
    CollisionSystem collisions;

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

                if (event.key.code == sf::Keyboard::T)
                    dayNight.nextMode();

                if (event.key.code == sf::Keyboard::R) {
                    // Restart: ambulance back at the hospital, traffic and
                    // signals back to their starting state, counters cleared.
                    ambulance.resetTo(startPosition, 0.0f);
                    traffic.reset();
                    trafficLights.reset();
                    collisions.reset();
                    redLightCamera.reset();
                    mission.restart(startPosition);
                }
            }
        }

        const float deltaTime = clock.restart().asSeconds();

        const Point beforeMove = ambulance.position();
        // Driving keys only count while the window has focus (the
        // keyboard state is global, so other apps would steer otherwise).
        ambulance.update(deltaTime, window.hasFocus() ? Ambulance::readKeyboard()
                                                      : DrivingInput{});
        redLightCamera.update(beforeMove, ambulance.position(), deltaTime);
        trafficLights.update(deltaTime);
        dayNight.update(deltaTime);
        pedestrians.update(deltaTime, trafficLights);

        // Traffic must not drive into the ambulance or people on a crossing.
        auto obstacles = pedestrians.footprints();
        obstacles.push_back(ambulance.footprint());
        traffic.update(deltaTime, trafficLights, obstacles);
        collisions.update(ambulance, city, traffic, deltaTime);
        mission.update(deltaTime, ambulance.position(), ambulance.speed());
        routeGuide.update(deltaTime, ambulance.position(), mission.incidentPosition(),
                          mission.state() == Mission::State::Active);

        window.clear(Colors::Background);

        city.draw(window, lineRenderer, graphicsDebug);
        environment.draw(window, graphicsDebug);
        trafficLights.draw(window, lineRenderer, graphicsDebug);
        routeGuide.draw(window, lineRenderer, mission.colour(), graphicsDebug);

        pedestrians.draw(window, lineRenderer, trafficLights, graphicsDebug);
        mission.draw(window, lineRenderer, graphicsDebug);
        traffic.draw(window, lineRenderer, curveDebug, graphicsDebug);
        ambulance.draw(window, lineRenderer, graphicsDebug);
        redLightCamera.draw(window, lineRenderer);
        collisions.draw(window, lineRenderer, graphicsDebug);

        // Night: tint the finished scene, then draw lights on top of it.
        dayNight.drawOverlay(window, lineRenderer);
        std::vector<DayNight::Light> lights;
        for (const auto& v : traffic.vehicles()) {
            const Point p = v.position();
            const Point d = v.direction();
            const float front = v.halfLength();
            lights.push_back({{p.x + d.x * front, p.y + d.y * front}, d,
                              sf::Color(255, 240, 180)});
        }
        {
            const float a = ambulance.heading() * 3.14159265f / 180.0f;
            const Point d{std::cos(a), std::sin(a)};
            const Point p = ambulance.position();
            lights.push_back({{p.x + d.x * 30.0f, p.y + d.y * 30.0f}, d,
                              sf::Color(255, 250, 220)});
        }
        for (const auto& [position, colour] : trafficLights.litBulbs())
            lights.push_back({position, {0.0f, 0.0f}, colour});
        dayNight.drawLights(window, lineRenderer, lights, graphicsDebug);

        clippingDemo.draw(window, lineRenderer, clippingDebug);

        window.display();
    }

    return 0;
}
