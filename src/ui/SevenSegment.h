#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include "../core/Point.h"
#include "../lines/LineRenderer.h"

// Seven-segment text drawn entirely with our own algorithms.
//
//      aaa
//     f   b       Each segment is a hexagon (a bar with pointed ends)
//     f   b       filled with the scan-line polygon fill. A character is
//      ggg        a 7-bit mask saying which segments are lit; ':' and '.'
//     e   c       are small squares. Digits, a minus sign and the letters
//     e   c       that seven segments can show (A b C d E F H I J L n o P
//      ddd        r S t U y) are supported; anything else is a blank.
namespace SevenSegment {

// Segment mask: bit 0 = a, 1 = b, 2 = c, 3 = d, 4 = e, 5 = f, 6 = g.
std::uint8_t maskFor(char c);

// Hexagons for the lit segments of one character, cell top-left at origin.
std::vector<std::vector<Point>> segments(char c, Point origin,
                                         float height, float thickness);

// Width of a cell including spacing (narrow cells for ':' and '.').
float advance(char c, float height);

float textWidth(const std::string& text, float height);

void drawText(sf::RenderTarget& target, const LineRenderer& lines,
              const std::string& text, Point origin, float height,
              sf::Color colour, sf::Color unlit = sf::Color::Transparent);

}
