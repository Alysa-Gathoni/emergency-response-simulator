#pragma once
#include "../core/Point.h"

// Compass direction of travel. Screen coordinates: +x is east, +y is south.
enum class Heading { North, East, South, West };

// Traffic streams that may move at the same time share a signal group.
enum class SignalGroup { NorthSouth, EastWest };

inline Point headingVector(Heading h) {
    switch (h) {
        case Heading::North: return {0.0f, -1.0f};
        case Heading::East:  return {1.0f, 0.0f};
        case Heading::South: return {0.0f, 1.0f};
        case Heading::West:  return {-1.0f, 0.0f};
    }
    return {};
}

// Left-hand normal of a direction vector. With y pointing down the screen,
// rotating d = (dx, dy) by -90 degrees gives (dy, -dx).
// Kenya drives on the LEFT, so a vehicle's lane centre is offset this way.
inline Point leftOf(Point d) { return {d.y, -d.x}; }

inline SignalGroup groupOf(Heading h) {
    return (h == Heading::North || h == Heading::South)
        ? SignalGroup::NorthSouth
        : SignalGroup::EastWest;
}
