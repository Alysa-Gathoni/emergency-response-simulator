#include "HelpPanel.h"
#include "PixelFont.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include <string>
#include <vector>

namespace {

const sf::Color Panel(12, 16, 22, 215);
const sf::Color Header(32, 40, 52, 235);
const sf::Color Frame(90, 100, 112);
const sf::Color Title(240, 200, 80);
const sf::Color Goal(240, 200, 80);
const sf::Color Key(120, 235, 180);
const sf::Color Action(220, 226, 232);
const sf::Color Hint(140, 150, 160);

constexpr int Scale = 2;                 // 5x7 glyphs drawn at 10x14
constexpr float Padding = 12.0f;
constexpr float RowHeight = 20.0f;
constexpr float GoalHeight = 18.0f;
constexpr float KeyColumn = 132.0f;      // 11 characters wide

const std::string Up(1, PixelFont::ArrowUp);
const std::string Down(1, PixelFont::ArrowDown);
const std::string LeftArrow(1, PixelFont::ArrowLeft);
const std::string RightArrow(1, PixelFont::ArrowRight);

const std::vector<std::string> GoalLines = {
    "GOAL: REACH THE FLASHING",
    "INCIDENT AND STOP ON IT",
    "BEFORE THE TIMER RUNS OUT.",
};

struct Row { std::string key; std::string action; };

const std::vector<Row> Rows = {
    {"W / " + Up,                          "ACCELERATE"},
    {"S / " + Down,                        "BRAKE, THEN REVERSE"},
    {"A D / " + LeftArrow + " " + RightArrow, "STEER"},
    {"SPACE",                              "HANDBRAKE"},
    {"P",                                  "PAUSE"},
    {"R",                                  "RESTART"},
    {"T",                                  "DAY / NIGHT"},
    {"G",                                  "DEBUG VIEW"},
    {"B",                                  "ROUTE CURVES"},
    {"C",                                  "CLIPPING DEMO"},
    {"L",                                  "DDA / BRESENHAM"},
    {"F12",                                "SCREENSHOT"},
    {"ESC",                                "QUIT"},
};

std::vector<Point> rect(float l, float t, float r, float b) {
    return {{l, t}, {r, t}, {r, b}, {l, b}};
}

// Text is 14 px tall; centre it vertically in a bar of height `h`.
float textTop(float top, float h) {
    return top + (h - PixelFont::GlyphHeight * Scale) * 0.5f;
}

}

float HelpPanel::height() const {
    if (!expanded_) return HeaderHeight;
    return HeaderHeight + Padding
         + GoalLines.size() * GoalHeight + Padding
         + Rows.size() * RowHeight + Padding * 0.5f;
}

bool HelpPanel::headerContains(Point p) const {
    return p.x >= Left && p.x <= Left + width()
        && p.y >= Top && p.y <= Top + HeaderHeight;
}

void HelpPanel::draw(sf::RenderTarget& target, const LineRenderer& lines) const {
    const float right = Left + width();
    const float bottom = Top + height();
    const float headerBottom = Top + HeaderHeight;

    // Background and title bar: scan-line filled rectangles.
    PolygonFill::draw(target, rect(Left, Top, right, bottom), lines, Panel, false);
    PolygonFill::draw(target, rect(Left, Top, right, headerBottom), lines, Header, false);
    Polygon(rect(Left, Top, right, bottom)).drawOutline(target, lines, Frame, 1.0f);

    const float headerText = textTop(Top, HeaderHeight);
    if (!expanded_) {
        PixelFont::drawText(target, "[H] SHOW CONTROLS", {Left + Padding, headerText},
                            Scale, Title);
        return;
    }

    PixelFont::drawText(target, "CONTROLS", {Left + Padding, headerText}, Scale, Title);
    const std::string hide = "[H] HIDE";
    PixelFont::drawText(target, hide,
                        {right - Padding - PixelFont::textWidth(hide, Scale), headerText},
                        Scale, Hint);
    lines.draw(target, {Left, headerBottom}, {right, headerBottom}, Frame, 1.0f);

    float y = headerBottom + Padding;
    for (const std::string& line : GoalLines) {
        PixelFont::drawText(target, line, {Left + Padding, y}, Scale, Goal);
        y += GoalHeight;
    }

    y += Padding * 0.5f;
    lines.draw(target, {Left + Padding, y}, {right - Padding, y}, Frame, 1.0f);
    y += Padding * 0.5f;

    for (const Row& row : Rows) {
        const float top = textTop(y, RowHeight);
        PixelFont::drawText(target, row.key, {Left + Padding, top}, Scale, Key);
        PixelFont::drawText(target, row.action, {Left + Padding + KeyColumn, top},
                            Scale, Action);
        y += RowHeight;
    }
}
