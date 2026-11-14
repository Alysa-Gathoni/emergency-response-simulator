#include "ObjectiveBanner.h"
#include "PixelFont.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include <algorithm>
#include <cmath>

namespace {

const Rgba Panel(12, 16, 22, 215);
const Rgba Frame(90, 100, 112);
const Rgba Instruction(230, 234, 238);
const Rgba Success(90, 230, 120);
const Rgba Failure(250, 90, 70);

const char* CallNames[] = {"MEDICAL CALL", "FIRE CALL", "ROAD CRASH"};
const Rgba CallColours[] = {{235, 70, 70}, {245, 150, 40}, {240, 210, 60}};

constexpr int Scale = 2;
constexpr float Padding = 14.0f;
constexpr float LineGap = 8.0f;
constexpr float CentreX = 550.0f;            // middle of the 1100 px window
constexpr float RightLimit = 862.0f;         // HUD panel starts at 870

std::vector<Point> rect(float l, float t, float r, float b) {
    return {{l, t}, {r, t}, {r, b}, {l, b}};
}

}

ObjectiveBanner::Lines ObjectiveBanner::text(const BannerData& d) {
    const int type = std::clamp(d.incidentType, 0, 2);
    switch (d.phase) {
        case BannerData::Phase::Arrived:
            return {std::string(d.delivered ? "PATIENT DELIVERED IN " : "ARRIVED IN ")
                        + std::to_string(static_cast<int>(std::lround(d.responseTime))) + " S",
                    "NEXT CALL COMING UP"};
        case BannerData::Phase::OnScene:
            return {std::string(CallNames[type]) + " - ON SCENE",
                    "CREW AT WORK, STAY STOPPED "
                        + std::to_string(static_cast<int>(std::lround(std::clamp(d.treatment, 0.0f, 1.0f) * 100.0f)))
                        + "%"};
        case BannerData::Phase::Transport: {
            const std::string headline = "TO HOSPITAL - "
                + std::to_string(std::max(0, d.metresToGo)) + " M TO GO";
            if (d.holding) return {headline, "HOLD STILL..."};
            if (d.nearScene) return {headline, "STOP IN THE HOSPITAL BAY"};
            return {headline, "PATIENT ON BOARD, FOLLOW THE ROUTE"};
        }
        case BannerData::Phase::Missed:
            return {"TOO LATE", "NEXT CALL COMING UP"};
        case BannerData::Phase::EnRoute:
            break;
    }

    const std::string headline = std::string(CallNames[type]) + " - "
                               + std::to_string(std::max(0, d.metresToGo)) + " M TO GO";
    if (d.holding) return {headline, "HOLD STILL..."};
    if (d.nearScene) return {headline, "SLOW DOWN AND STOP ON THE MARKER"};
    return {headline, "FOLLOW THE ROUTE, STOP AT THE MARKER"};
}

void ObjectiveBanner::draw(IRenderer& target, const LineRenderer& lines,
                           const BannerData& d, float leftLimit) const {
    const Lines t = text(d);
    const float textW = std::max(PixelFont::textWidth(t.headline, Scale),
                                 PixelFont::textWidth(t.instruction, Scale));
    const float width = textW + Padding * 2.0f;
    const float glyphH = static_cast<float>(PixelFont::GlyphHeight * Scale);
    const float height = Padding * 2.0f + glyphH * 2.0f + LineGap;

    // Centred, but pushed right of the controls panel and kept off the HUD.
    float left = std::max(CentreX - width * 0.5f, leftLimit);
    left = std::min(left, RightLimit - width);
    const float right = left + width;
    const float bottom = Top + height;

    PolygonFill::draw(target, rect(left, Top, right, bottom), lines, Panel, false);
    Polygon(rect(left, Top, right, bottom)).drawOutline(target, lines, Frame, 1.0f);

    Rgba headlineColour = CallColours[std::clamp(d.incidentType, 0, 2)];
    if (d.phase == BannerData::Phase::Transport) headlineColour = Rgba(90, 170, 255);
    if (d.phase == BannerData::Phase::Arrived) headlineColour = Success;
    if (d.phase == BannerData::Phase::Missed) headlineColour = Failure;

    const float y1 = Top + Padding;
    const float y2 = y1 + glyphH + LineGap;
    PixelFont::drawText(target, t.headline,
                        {left + (width - PixelFont::textWidth(t.headline, Scale)) * 0.5f, y1},
                        Scale, headlineColour);
    PixelFont::drawText(target, t.instruction,
                        {left + (width - PixelFont::textWidth(t.instruction, Scale)) * 0.5f, y2},
                        Scale, d.holding ? Success : Instruction);
}
