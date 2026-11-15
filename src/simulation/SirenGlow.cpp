#include "SirenGlow.h"
#include "../shapes/CircleEllipse.h"
#include <algorithm>
#include <cmath>

namespace SirenGlow {

int discAlpha(float darkness) {
    const float d = std::clamp(darkness, 0.0f, 1.0f);
    return static_cast<int>(std::lround(DayAlpha + (NightAlpha - DayAlpha) * d));
}

void draw(IRenderer& target, const Ambulance& ambulance, float darkness) {
    if (!ambulance.sirenOn()) return;
    const bool red = ambulance.sirenPhase() == 0;
    const Point c = red ? ambulance.redLightPosition() : ambulance.blueLightPosition();
    const Rgba base = red ? Rgba(255, 70, 60) : Rgba(70, 130, 255);
    const Rgba colour(base.r, base.g, base.b, discAlpha(darkness));

    // Largest disc first: the small bright core is drawn last, on top.
    for (int k = Rings - 1; k >= 0; --k)
        ShapeAlgorithms::drawPoints(target,
                                    ShapeAlgorithms::filledCircle(c, InnerRadius + k * RingStep),
                                    colour, 1.0f);
}

}
