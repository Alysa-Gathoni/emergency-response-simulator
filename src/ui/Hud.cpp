#include "Hud.h"
#include "SevenSegment.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include "../shapes/CircleEllipse.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

const sf::Color Panel(12, 16, 22, 200);
const sf::Color Frame(90, 100, 112);
const sf::Color Label(120, 140, 150);
const sf::Color Value(120, 235, 180);
const sf::Color Warning(250, 90, 70);
const sf::Color Unlit(40, 60, 54, 150);

const char* IncidentNames[] = {"AID", "FIRE", "CRASH"};
const sf::Color IncidentColours[] = {
    {235, 70, 70}, {245, 150, 40}, {240, 210, 60}
};

std::vector<Point> rect(float l, float t, float r, float b) {
    return {{l, t}, {r, t}, {r, b}, {l, b}};
}

void row(sf::RenderTarget& target, const LineRenderer& lines,
         float x, float y, const std::string& label, const std::string& value,
         sf::Color colour) {
    SevenSegment::drawText(target, lines, label, {x, y + 4}, 12.0f, Label);
    SevenSegment::drawText(target, lines, value, {x + 70, y}, 18.0f, colour, Unlit);
}

}

std::string Hud::clock(float seconds) {
    const int s = std::max(0, static_cast<int>(std::ceil(seconds)));
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", s / 60, s % 60);
    return buffer;
}

int Hud::kilometresPerHour(float pixelsPerSecond) {
    return static_cast<int>(std::lround(std::abs(pixelsPerSecond) * MetresPerPixel * 3.6f));
}

int Hud::metres(float pixels) {
    return static_cast<int>(std::lround(pixels * MetresPerPixel));
}

void Hud::draw(sf::RenderTarget& target, const LineRenderer& lines,
               const HudData& d) const {
    // Translucent panel in the top-right corner (scan-line fill + outline).
    const float left = 870, top = 8, right = 1092, bottom = 206;
    PolygonFill::draw(target, rect(left, top, right, bottom), lines, Panel, false);
    Polygon(rect(left, top, right, bottom)).drawOutline(target, lines, Frame, 1.0f);

    const int type = std::clamp(d.incidentType, 0, 2);
    const float x = left + 12;
    float y = top + 12;

    // Incident type with its marker colour.
    ShapeAlgorithms::drawPoints(target,
                                ShapeAlgorithms::filledCircle({x + 6, y + 9}, 6),
                                IncidentColours[type], 1.0f);
    SevenSegment::drawText(target, lines, IncidentNames[type], {x + 22, y}, 18.0f,
                           IncidentColours[type]);
    y += 32;

    const bool hurry = d.timeLeft < 10.0f;
    row(target, lines, x, y, "LEFT", clock(d.timeLeft), hurry ? Warning : Value);
    y += 28;
    row(target, lines, x, y, "DIST", std::to_string(metres(d.distance)), Value);
    y += 28;
    row(target, lines, x, y, "SPD", std::to_string(kilometresPerHour(d.speed)), Value);
    y += 28;
    row(target, lines, x, y, "COL", std::to_string(d.collisions),
        d.collisions > 0 ? Warning : Value);
    SevenSegment::drawText(target, lines, "RED", {x + 118, y + 4}, 12.0f, Label);
    SevenSegment::drawText(target, lines, std::to_string(d.redLights), {x + 160, y},
                           18.0f, d.redLights > 0 ? Warning : Value, Unlit);
    y += 28;
    row(target, lines, x, y, "DONE", std::to_string(d.completed) + "-" + std::to_string(d.missed),
        Value);

    // Time bar under the panel: shrinks as the time limit runs out.
    const float fraction = std::clamp(d.timeLeft / std::max(1.0f, d.timeLimit), 0.0f, 1.0f);
    PolygonFill::draw(target, rect(left, bottom + 4, left + (right - left) * fraction, bottom + 9),
                      lines, hurry ? Warning : Value, false);

    // Line algorithm currently in use (L key), bottom-left corner.
    SevenSegment::drawText(target, lines, d.bresenham ? "BRES" : "DDA", {12, 694}, 14.0f,
                           Label);

    // Result banner in the middle of the screen.
    if (d.arrived || d.late) {
        const std::string text = d.arrived ? "DONE" : "LATE";
        const float h = 48.0f;
        const float w = SevenSegment::textWidth(text, h);
        const float cx = 550.0f - w * 0.5f;
        PolygonFill::draw(target, rect(cx - 20, 300, cx + w + 20, 300 + h + 30), lines,
                          Panel, false);
        SevenSegment::drawText(target, lines, text, {cx, 315}, h,
                               d.arrived ? Value : Warning);
    }
}
