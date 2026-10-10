#pragma once
#include <SFML/Graphics.hpp>

class Environment {
public:
    void draw(sf::RenderTarget& target, bool debug) const;
};
