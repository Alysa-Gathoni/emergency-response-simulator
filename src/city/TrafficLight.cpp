#include "TrafficLight.h"
#include "../shapes/CircleEllipse.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include "../core/Color.h"

namespace {

sf::Color bulbColor(Signal lit, Signal bulb) {
    if (lit != bulb) return sf::Color(70, 70, 70);
    switch (bulb) {
        case Signal::Red:    return sf::Color(235, 70, 70);
        case Signal::Yellow: return sf::Color(240, 205, 70);
        case Signal::Green:  return sf::Color(80, 215, 105);
    }
    return sf::Color::White;
}

Point offset(Point p, Point d, float s) { return {p.x + d.x * s, p.y + d.y * s}; }

}

TrafficLights::TrafficLights(const RoadNetwork& network)
    : network_(network) {
    reset();
}

void TrafficLights::reset() {
    // Offsets in seconds: the diagonal neighbours switch together and the
    // others are shifted by a third of a cycle, so the grid never switches
    // all at once and a vehicle often meets green at the next junction.
    const float offsets[] = {0.0f, 8.0f, 8.0f, 16.0f};
    const auto& nodes = network_.intersections();
    controllers_.clear();
    for (std::size_t i = 0; i < nodes.size(); ++i)
        controllers_.emplace_back(offsets[i % 4]);
}

void TrafficLights::update(float deltaTime) {
    for (auto& c : controllers_)
        c.update(deltaTime);
}

Signal TrafficLights::signalAt(int intersection, SignalGroup group) const {
    return controllers_[static_cast<std::size_t>(intersection)].signalFor(group);
}

Signal TrafficLights::signalFor(const StopLine& stopLine) const {
    return signalAt(stopLine.intersection, groupOf(stopLine.approach));
}

const SignalController& TrafficLights::controller(int intersection) const {
    return controllers_[static_cast<std::size_t>(intersection)];
}

void TrafficLights::draw(sf::RenderTarget& target,
                         const LineRenderer& lines,
                         bool debug) const {
    for (const StopLine& stop : network_.stopLines()) {
        const Signal lit = signalFor(stop);
        const Point d = headingVector(stop.approach);
        const Point side = leftOf(d);

        // Signal head on the left kerb, just before the stop line. The
        // housing is built in the approach's local frame (d, side) so the
        // three bulbs line up with the direction of travel.
        const Point base = offset(stop.b, side, 12.0f);
        const Point centre = offset(base, d, -14.0f);

        Polygon housing({
            offset(offset(centre, d, -16.0f), side, -7.0f),
            offset(offset(centre, d,  16.0f), side, -7.0f),
            offset(offset(centre, d,  16.0f), side,  7.0f),
            offset(offset(centre, d, -16.0f), side,  7.0f)
        });
        PolygonFill::draw(target, housing.vertices(), lines,
                          sf::Color(28, 28, 30), false);
        housing.drawOutline(target, lines, sf::Color(170, 170, 170), 1.0f);

        // Red nearest the junction, green furthest from it.
        const Point red = offset(centre, d, 10.0f);
        const Point green = offset(centre, d, -10.0f);
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(red, 4),
                                    bulbColor(lit, Signal::Red), 1.0f);
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(centre, 4),
                                    bulbColor(lit, Signal::Yellow), 1.0f);
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(green, 4),
                                    bulbColor(lit, Signal::Green), 1.0f);

        if (debug) {
            // Stop line coloured with the signal that governs it.
            lines.draw(target, stop.a, stop.b, bulbColor(lit, lit), 3.0f);
        }
    }

    if (!debug) return;

    // Remaining time of the current phase as a shrinking bar per junction.
    for (std::size_t i = 0; i < controllers_.size(); ++i) {
        const auto& c = controllers_[i];
        const Point centre = network_.intersections()[i].centre;
        const float fraction = c.timeUntilChange() / c.phaseDuration(c.phase());
        const Point start{centre.x - 30.0f, centre.y};
        lines.draw(target, start, {start.x + 60.0f * fraction, start.y},
                   Colors::Debug, 3.0f);
    }
}
