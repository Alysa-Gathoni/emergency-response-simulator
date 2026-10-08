#pragma once
#include <SFML/Graphics.hpp>
#include "LineAlgorithms.h"

enum class LineMode { Bresenham, DDA };

class LineRenderer {
public:
    explicit LineRenderer(LineMode mode = LineMode::Bresenham);

    void setMode(LineMode mode);
    LineMode getMode() const;
    void setDebug(bool enabled);
    bool isDebug() const;

    void draw(sf::RenderTarget& target, Point start, Point end,
              sf::Color color, float pixelSize = 2.0f) const;

private:
    LineMode mode_;
    bool debug_{false};
};
