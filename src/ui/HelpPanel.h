#pragma once
#include "render/IRenderer.h"
#include "../core/Point.h"
#include "../lines/LineRenderer.h"

// On-screen controls panel in the top-left corner.
//
// It starts open so a new trainee sees the goal and the keys straight
// away. H (or a click on its title bar) shrinks it to a small tab and
// opens it again, so it does not cover the map while driving.
class HelpPanel {
public:
    static constexpr float Left = 8.0f;
    static constexpr float Top = 8.0f;
    static constexpr float Width = 384.0f;
    static constexpr float TabWidth = 226.0f;
    static constexpr float HeaderHeight = 30.0f;

    bool expanded() const { return expanded_; }
    void setExpanded(bool expanded) { expanded_ = expanded; }
    void toggle() { expanded_ = !expanded_; }

    // Height of the whole panel when open, or of the tab when minimised.
    float height() const;
    float width() const { return expanded_ ? Width : TabWidth; }

    // True when `p` is on the title bar (or the tab): clicking it toggles.
    bool headerContains(Point p) const;

    void draw(IRenderer& target, const LineRenderer& lines) const;

private:
    bool expanded_{true};
};
