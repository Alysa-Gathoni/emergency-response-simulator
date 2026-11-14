#pragma once
#include <vector>
#include "RoadNetwork.h"
#include "render/IRenderer.h"

// Trees in the parks: a grid of canopies about 64 px apart with a fixed
// jitter, so every park of the generated map is planted automatically.
class Environment {
public:
    explicit Environment(const std::vector<Rect>& parks);
    void draw(IRenderer& target, bool debug) const;

private:
    std::vector<Point> trees_;
};
