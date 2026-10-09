#include <SFML/Graphics.hpp>
#include "core/Color.h"
#include "lines/LineRenderer.h"
#include "city/City.h"
#include "simulation/Ambulance.h"
#include "simulation/ClippingDemo.h"

int main() {
    sf::RenderWindow window(
        sf::VideoMode(900, 700),
        "Emergency Response Simulator - Chunk 3"
    );
    window.setFramerateLimit(60);

    LineRenderer lineRenderer(LineMode::Bresenham);
    City city;
    Ambulance ambulance({150.0f, 310.0f});
    ClippingDemo clippingDemo;

    bool graphicsDebug = false;
    bool clippingDebug = false;
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
            }
        }

        const float deltaTime = clock.restart().asSeconds();
        ambulance.update(deltaTime);

        window.clear(Colors::Background);
        city.draw(window, lineRenderer, graphicsDebug);
        ambulance.draw(window, lineRenderer, graphicsDebug);
        clippingDemo.draw(window, lineRenderer, clippingDebug);
        window.display();
    }

    return 0;
}
