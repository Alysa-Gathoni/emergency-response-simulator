#pragma once
#include "render/IRenderer.h"
#include "Ambulance.h"

// Soft red/blue glow round the lit siren light, strongest at night.
//
// A radial falloff is approximated by Rings concentric discs (midpoint
// circle, row-filled) drawn on top of each other with a small alpha each:
// a point at distance d from the light is covered by every disc with
// radius > d, so with alpha blending its brightness falls off towards the
// edge:  coverage(d) = 1 - (1 - a)^(number of discs with r_k > d).
//   r_k = InnerRadius + k * RingStep,   a = alpha per disc
// The per-disc alpha scales with the darkness of the day/night cycle.
namespace SirenGlow {

constexpr int Rings = 6;
constexpr int InnerRadius = 6;
constexpr int RingStep = 7;
constexpr int DayAlpha = 14;     // per disc, full daylight
constexpr int NightAlpha = 40;   // per disc, full night

// Alpha of one disc at the given darkness (0 day .. 1 night).
int discAlpha(float darkness);

void draw(IRenderer& target, const Ambulance& ambulance, float darkness);

}
