#include <SFML/Graphics.hpp>
#include "core/Color.h"
#include "lines/LineRenderer.h"
#include "city/RoadNetwork.h"
#include "city/City.h"
#include "city/TrafficLight.h"
#include "city/RedLightCamera.h"
#include "city/Environment.h"
#include "simulation/Ambulance.h"
#include "simulation/ClippingDemo.h"
#include "simulation/CollisionSystem.h"
#include "simulation/Mission.h"
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
    TrafficLights trafficLights(network);
    Ambulance ambulance(startPosition);
    ClippingDemo clippingDemo;
    Mission mission;
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
        traffic.update(deltaTime, trafficLights, {ambulance.footprint()});
        collisions.update(ambulance, city, traffic, deltaTime);
        mission.update(deltaTime, ambulance.position(), ambulance.speed());

        window.clear(Colors::Background);

        city.draw(window, lineRenderer, graphicsDebug);
        environment.draw(window, graphicsDebug);
        trafficLights.draw(window, lineRenderer, graphicsDebug);
        mission.draw(window, lineRenderer, graphicsDebug);
        traffic.draw(window, lineRenderer, curveDebug, graphicsDebug);
        ambulance.draw(window, lineRenderer, graphicsDebug);
        redLightCamera.draw(window, lineRenderer);
        collisions.draw(window, lineRenderer, graphicsDebug);
        clippingDemo.draw(window, lineRenderer, clippingDebug);

        window.display();
    }

    return 0;
}
