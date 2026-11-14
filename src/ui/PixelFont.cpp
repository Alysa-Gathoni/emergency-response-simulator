#include "PixelFont.h"
#include "../core/Raster.h"
#include <cctype>

namespace PixelFont {

Glyph glyph(char c) {
    switch (static_cast<char>(std::toupper(static_cast<unsigned char>(c)))) {
        case 'A': return {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
        case 'B': return {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110};
        case 'C': return {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110};
        case 'D': return {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110};
        case 'E': return {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111};
        case 'F': return {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000};
        case 'G': return {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111};
        case 'H': return {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
        case 'I': return {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
        case 'J': return {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100};
        case 'K': return {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001};
        case 'L': return {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111};
        case 'M': return {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001};
        case 'N': return {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001};
        case 'O': return {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
        case 'P': return {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000};
        case 'Q': return {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101};
        case 'R': return {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001};
        case 'S': return {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110};
        case 'T': return {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
        case 'U': return {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
        case 'V': return {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100};
        case 'W': return {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010};
        case 'X': return {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001};
        case 'Y': return {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100};
        case 'Z': return {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111};
        case '0': return {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110};
        case '1': return {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
        case '2': return {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111};
        case '3': return {0b11111, 0b00010, 0b00100, 0b00010, 0b00001, 0b10001, 0b01110};
        case '4': return {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010};
        case '5': return {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110};
        case '6': return {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110};
        case '7': return {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000};
        case '8': return {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110};
        case '9': return {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100};
        case '/': return {0b00001, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b10000};
        case '-': return {0b00000, 0b00000, 0b00000, 0b01110, 0b00000, 0b00000, 0b00000};
        case '+': return {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000};
        case '.': return {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01100};
        case ',': return {0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b00100, 0b01000};
        case ':': return {0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b01100, 0b00000};
        case '!': return {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100};
        case '?': return {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100};
        case '(': return {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010};
        case ')': return {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000};
        case '[': return {0b01110, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01110};
        case ']': return {0b01110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01110};
        case ArrowUp:    return {0b00100, 0b01110, 0b10101, 0b00100, 0b00100, 0b00100, 0b00100};
        case ArrowDown:  return {0b00100, 0b00100, 0b00100, 0b00100, 0b10101, 0b01110, 0b00100};
        case ArrowLeft:  return {0b00000, 0b00100, 0b01000, 0b11111, 0b01000, 0b00100, 0b00000};
        case ArrowRight: return {0b00000, 0b00100, 0b00010, 0b11111, 0b00010, 0b00100, 0b00000};
        default: return {};
    }
}

float textWidth(const std::string& text, int scale) {
    if (text.empty()) return 0.0f;
    return static_cast<float>((static_cast<int>(text.size()) * Advance - 1) * scale);
}

std::vector<std::pair<Point, Point>> spans(const std::string& text,
                                           Point origin, int scale) {
    std::vector<std::pair<Point, Point>> result;
    float x = origin.x;
    for (char c : text) {
        const Glyph g = glyph(c);
        for (int row = 0; row < GlyphHeight; ++row) {
            // Walk the row's bits left to right, emitting one run per
            // stretch of consecutive lit columns.
            int col = 0;
            while (col < GlyphWidth) {
                if (!(g[row] & (1 << (GlyphWidth - 1 - col)))) { ++col; continue; }
                const int start = col;
                while (col < GlyphWidth && (g[row] & (1 << (GlyphWidth - 1 - col))))
                    ++col;
                const float x1 = x + static_cast<float>(start * scale);
                const float x2 = x + static_cast<float>(col * scale - 1);
                for (int k = 0; k < scale; ++k) {
                    const float y = origin.y + static_cast<float>(row * scale + k);
                    result.push_back({{x1, y}, {x2, y}});
                }
            }
        }
        x += static_cast<float>(Advance * scale);
    }
    return result;
}

void drawText(sf::RenderTarget& target, const std::string& text,
              Point origin, int scale, sf::Color colour) {
    Raster::spans(target, spans(text, origin, scale), colour);
}

}
