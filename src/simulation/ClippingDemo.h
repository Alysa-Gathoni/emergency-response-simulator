#pragma once
#include <SFML/Graphics.hpp>
#include "../lines/LineRenderer.h"
#include "../rasterization/Clipping.h"

class ClippingDemo {
public:
    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              bool enabled) const;

private:
    ClipRect viewport_{90.0f, 40.0f, 810.0f, 660.0f};
};
