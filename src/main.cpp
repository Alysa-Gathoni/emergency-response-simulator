#include <SFML/Graphics.hpp>
#include <string>
#include "city/City.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(960, 640), "Emergency Response Simulator - Chunk 1");
    window.setFramerateLimit(60);
    City city;
    bool lWasDown = false, gWasDown = false;

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) window.close();
        }
        const bool lDown = sf::Keyboard::isKeyPressed(sf::Keyboard::L);
        if (lDown && !lWasDown) city.setAlgorithm(city.algorithm() == LineAlgorithm::Bresenham ? LineAlgorithm::DDA : LineAlgorithm::Bresenham);
        lWasDown = lDown;
        const bool gDown = sf::Keyboard::isKeyPressed(sf::Keyboard::G);
        if (gDown && !gWasDown) city.setDebug(!city.debug());
        gWasDown = gDown;

        const std::string algorithm = city.algorithm() == LineAlgorithm::Bresenham ? "Bresenham" : "DDA";
        window.setTitle("Emergency Response Simulator | Chunk 1 | " + algorithm + " | L: algorithm  G: debug  Esc: quit");
        window.clear(sf::Color(30, 38, 43));
        city.draw(window);
        window.display();
    }
    return 0;
}
