#pragma once
#include "render/IRenderer.h"

class Environment {
public:
    void draw(IRenderer& target, bool debug) const;
};
