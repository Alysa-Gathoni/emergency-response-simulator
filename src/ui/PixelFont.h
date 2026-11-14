#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "../core/Point.h"

// A hand-made 5x7 bitmap font for readable text (the seven-segment font
// cannot show letters such as M, W or K).
//
// Each glyph is 7 rows of 5 bits, bit 4 being the leftmost column. A row
// is turned into horizontal runs of lit pixels and every run becomes one
// scan-line span, so a whole string is a single batched draw. Lower-case
// letters are drawn as capitals; arrows use the codes below.
namespace PixelFont {

constexpr int GlyphWidth = 5;
constexpr int GlyphHeight = 7;
constexpr int Advance = 6;          // glyph width plus a one-column gap

constexpr char ArrowUp = '\x01';
constexpr char ArrowDown = '\x02';
constexpr char ArrowLeft = '\x03';
constexpr char ArrowRight = '\x04';

using Glyph = std::array<std::uint8_t, GlyphHeight>;

// Bit rows of one character (all zero for unsupported characters).
Glyph glyph(char c);

// Width in pixels of `text` drawn at integer `scale` (no trailing gap).
float textWidth(const std::string& text, int scale);

// Scan-line spans (x1..x2 inclusive on row y) covering every lit pixel.
std::vector<std::pair<Point, Point>> spans(const std::string& text,
                                           Point origin, int scale);

void drawText(sf::RenderTarget& target, const std::string& text,
              Point origin, int scale, sf::Color colour);

}
