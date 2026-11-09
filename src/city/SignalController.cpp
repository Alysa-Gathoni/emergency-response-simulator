#include "SignalController.h"
#include <cmath>

namespace {

SignalController::Phase nextPhase(SignalController::Phase p) {
    using P = SignalController::Phase;
    switch (p) {
        case P::NorthSouthGreen:   return P::NorthSouthYellow;
        case P::NorthSouthYellow:  return P::ClearToEastWest;
        case P::ClearToEastWest:   return P::EastWestGreen;
        case P::EastWestGreen:     return P::EastWestYellow;
        case P::EastWestYellow:    return P::ClearToNorthSouth;
        case P::ClearToNorthSouth: return P::NorthSouthGreen;
    }
    return P::NorthSouthGreen;
}

}

SignalController::SignalController(float offsetSeconds, SignalTiming timing)
    : timing_(timing) {
    // Start part-way through the cycle instead of simulating the offset.
    const float cycle = cycleLength();
    if (cycle > 0.0f) update(std::fmod(std::abs(offsetSeconds), cycle));
}

void SignalController::update(float deltaTime) {
    elapsed_ += deltaTime;
    // A while loop keeps the controller correct even after a long frame.
    while (elapsed_ >= phaseDuration(phase_)) {
        elapsed_ -= phaseDuration(phase_);
        phase_ = nextPhase(phase_);
    }
}

Signal SignalController::signalFor(SignalGroup group) const {
    const bool northSouth = group == SignalGroup::NorthSouth;
    switch (phase_) {
        case Phase::NorthSouthGreen:  return northSouth ? Signal::Green : Signal::Red;
        case Phase::NorthSouthYellow: return northSouth ? Signal::Yellow : Signal::Red;
        case Phase::EastWestGreen:    return northSouth ? Signal::Red : Signal::Green;
        case Phase::EastWestYellow:   return northSouth ? Signal::Red : Signal::Yellow;
        default:                      return Signal::Red;
    }
}

SignalController::Phase SignalController::phase() const { return phase_; }
float SignalController::timeInPhase() const { return elapsed_; }

float SignalController::phaseDuration(Phase phase) const {
    switch (phase) {
        case Phase::NorthSouthGreen:
        case Phase::EastWestGreen:    return timing_.green;
        case Phase::NorthSouthYellow:
        case Phase::EastWestYellow:   return timing_.yellow;
        default:                      return timing_.allRed;
    }
}

float SignalController::timeUntilChange() const {
    return phaseDuration(phase_) - elapsed_;
}

float SignalController::cycleLength() const {
    return 2.0f * (timing_.green + timing_.yellow + timing_.allRed);
}
