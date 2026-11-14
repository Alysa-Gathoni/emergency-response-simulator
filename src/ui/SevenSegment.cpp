#include "SevenSegment.h"
#include "../rasterization/PolygonFill.h"
#include <cctype>

namespace {

enum : std::uint8_t {
    A = 1 << 0, B = 1 << 1, C = 1 << 2, D = 1 << 3,
    E = 1 << 4, F = 1 << 5, G = 1 << 6
};

// Horizontal bar from x0 to x1 centred on y, pointed at both ends.
std::vector<Point> horizontal(float x0, float x1, float y, float t) {
    const float h = t * 0.5f;
    return {{x0, y}, {x0 + h, y - h}, {x1 - h, y - h},
            {x1, y}, {x1 - h, y + h}, {x0 + h, y + h}};
}

// Vertical bar from y0 to y1 centred on x.
std::vector<Point> vertical(float x, float y0, float y1, float t) {
    const float h = t * 0.5f;
    return {{x, y0}, {x + h, y0 + h}, {x + h, y1 - h},
            {x, y1}, {x - h, y1 - h}, {x - h, y0 + h}};
}

}

namespace SevenSegment {

std::uint8_t maskFor(char c) {
    switch (std::toupper(static_cast<unsigned char>(c))) {
        case '0': return A | B | C | D | E | F;
        case '1': return B | C;
        case '2': return A | B | G | E | D;
        case '3': return A | B | G | C | D;
        case '4': return F | G | B | C;
        case '5': return A | F | G | C | D;
        case '6': return A | F | G | E | C | D;
        case '7': return A | B | C;
        case '8': return A | B | C | D | E | F | G;
        case '9': return A | B | C | D | F | G;
        case 'A': return A | B | C | E | F | G;
        case 'B': return F | E | G | C | D;          // b
        case 'C': return A | F | E | D;
        case 'D': return B | C | D | E | G;          // d
        case 'E': return A | F | G | E | D;
        case 'F': return A | F | G | E;
        case 'H': return F | E | G | B | C;
        case 'I': return E | F;
        case 'J': return B | C | D | E;
        case 'L': return F | E | D;
        case 'N': return E | G | C;                  // n
        case 'O': return E | G | C | D;              // o
        case 'P': return A | B | F | G | E;
        case 'R': return E | G;                      // r
        case 'S': return A | F | G | C | D;
        case 'T': return F | E | G | D;              // t
        case 'U': return F | E | D | C | B;
        case 'Y': return F | G | B | C | D;          // y
        case '-': return G;
        default:  return 0;
    }
}

std::vector<std::vector<Point>> segments(char c, Point o, float height, float t) {
    std::vector<std::vector<Point>> result;
    const float w = height * 0.55f;
    const float mid = o.y + height * 0.5f;

    if (c == ':') {
        for (float fy : {0.3f, 0.7f}) {
            const float y = o.y + height * fy;
            result.push_back({{o.x, y - t * 0.5f}, {o.x + t, y - t * 0.5f},
                              {o.x + t, y + t * 0.5f}, {o.x, y + t * 0.5f}});
        }
        return result;
    }
    if (c == '.') {
        const float y = o.y + height;
        result.push_back({{o.x, y - t}, {o.x + t, y - t}, {o.x + t, y}, {o.x, y}});
        return result;
    }

    const std::uint8_t m = maskFor(c);
    const float gap = t * 0.15f;   // tiny gap so segments read as separate
    if (m & A) result.push_back(horizontal(o.x + gap, o.x + w - gap, o.y, t));
    if (m & G) result.push_back(horizontal(o.x + gap, o.x + w - gap, mid, t));
    if (m & D) result.push_back(horizontal(o.x + gap, o.x + w - gap, o.y + height, t));
    if (m & F) result.push_back(vertical(o.x, o.y + gap, mid - gap, t));
    if (m & B) result.push_back(vertical(o.x + w, o.y + gap, mid - gap, t));
    if (m & E) result.push_back(vertical(o.x, mid + gap, o.y + height - gap, t));
    if (m & C) result.push_back(vertical(o.x + w, mid + gap, o.y + height - gap, t));
    return result;
}

float advance(char c, float height) {
    const float t = height * 0.14f;
    if (c == ':' || c == '.') return t * 2.5f;
    return height * 0.55f + t * 2.2f;
}

float textWidth(const std::string& text, float height) {
    float w = 0.0f;
    for (char c : text) w += advance(c, height);
    return w;
}

void drawText(sf::RenderTarget& target, const LineRenderer& lines,
              const std::string& text, Point origin, float height,
              sf::Color colour, sf::Color unlit) {
    const float t = height * 0.14f;
    Point cursor{origin.x + t * 0.5f, origin.y};

    for (char c : text) {
        if (unlit.a > 0 && c != ':' && c != '.' && c != ' ') {
            // Faint "off" segments, like a real display.
            for (const auto& seg : segments('8', cursor, height, t))
                PolygonFill::draw(target, seg, lines, unlit, false);
        }
        for (const auto& seg : segments(c, cursor, height, t))
            PolygonFill::draw(target, seg, lines, colour, false);
        cursor.x += advance(c, height);
    }
}

}
