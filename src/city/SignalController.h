#pragma once
#include "Direction.h"

enum class Signal { Red, Yellow, Green };

struct SignalTiming {
    float green{8.0f};
    float yellow{2.5f};
    float allRed{1.5f};   // clearance: both groups red before switching
};

// Finite-state machine for one intersection with two signal groups.
//
//   NS_GREEN -> NS_YELLOW -> ALL_RED -> EW_GREEN -> EW_YELLOW -> ALL_RED -> ...
//
// Only one group can ever be green or yellow, so conflicting streams never
// move together. The all-red phase lets vehicles that entered on yellow
// clear the box. A per-intersection offset staggers neighbouring
// controllers so the grid does not switch all at once (a simple green wave).
class SignalController {
public:
    enum class Phase {
        NorthSouthGreen,
        NorthSouthYellow,
        ClearToEastWest,
        EastWestGreen,
        EastWestYellow,
        ClearToNorthSouth
    };

    explicit SignalController(float offsetSeconds = 0.0f,
                              SignalTiming timing = {});

    void update(float deltaTime);

    Signal signalFor(SignalGroup group) const;
    Phase phase() const;
    float timeInPhase() const;
    float phaseDuration(Phase phase) const;
    float timeUntilChange() const;
    float cycleLength() const;

private:
    SignalTiming timing_;
    Phase phase_{Phase::NorthSouthGreen};
    float elapsed_{0.0f};
};
