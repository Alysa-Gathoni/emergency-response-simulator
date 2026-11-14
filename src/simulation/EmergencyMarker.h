#pragma once
#include "render/IRenderer.h"
#include "../core/Point.h"

class EmergencyMarker {
public:
    explicit EmergencyMarker(Point position);

    void update(float deltaTime);
    void draw(IRenderer& target, bool debug) const;

    Point position() const;
    void setPosition(Point position);
    void setColour(Rgba colour);

private:
    Point position_;
    float phase_{0.0f};
    Rgba colour_{235, 70, 70};
};
