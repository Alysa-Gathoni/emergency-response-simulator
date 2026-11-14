#pragma once
#include <cstdint>

// 8-bit RGBA colour used by every module, so scene code never needs a
// graphics library header. The renderer backends convert it to whatever
// their API expects.
struct Rgba {
    std::uint8_t r{0};
    std::uint8_t g{0};
    std::uint8_t b{0};
    std::uint8_t a{255};

    constexpr Rgba() = default;
    constexpr Rgba(int red, int green, int blue, int alpha = 255)
        : r(static_cast<std::uint8_t>(red)),
          g(static_cast<std::uint8_t>(green)),
          b(static_cast<std::uint8_t>(blue)),
          a(static_cast<std::uint8_t>(alpha)) {}

    constexpr bool operator==(const Rgba& o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
    constexpr bool operator!=(const Rgba& o) const { return !(*this == o); }

    static const Rgba White;
    static const Rgba Black;
    static const Rgba Red;
    static const Rgba Blue;
    static const Rgba Transparent;
};

inline constexpr Rgba Rgba::White{255, 255, 255};
inline constexpr Rgba Rgba::Black{0, 0, 0};
inline constexpr Rgba Rgba::Red{255, 0, 0};
inline constexpr Rgba Rgba::Blue{0, 0, 255};
inline constexpr Rgba Rgba::Transparent{0, 0, 0, 0};
