#pragma once
#include "render/IRenderer.h"
#include "../lines/LineRenderer.h"
#include "../rasterization/Clipping.h"

class ClippingDemo {
public:
    void draw(IRenderer& target,
              const LineRenderer& lines,
              bool enabled) const;

private:
    ClipRect viewport_{90.0f, 40.0f, 810.0f, 660.0f};
};
