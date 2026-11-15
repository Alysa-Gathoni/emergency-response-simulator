#include "Check.h"
#include "ui/HelpPanel.h"
#include "ui/ObjectiveBanner.h"
#include "simulation/Incidents.h"
#include "ui/PixelFont.h"

namespace {

void glyphs() {
    // Middle bar of an A is fully lit; a T has only its top row full.
    CHECK(PixelFont::glyph('A')[3] == 0b11111);
    CHECK(PixelFont::glyph('T')[0] == 0b11111);
    CHECK(PixelFont::glyph('T')[1] == 0b00100);
    CHECK(PixelFont::glyph('m') == PixelFont::glyph('M'));   // lower case drawn as capitals
    CHECK(PixelFont::glyph('~') == PixelFont::Glyph{});       // unsupported -> blank
    CHECK(PixelFont::glyph(' ') == PixelFont::Glyph{});
}

void widthsAndSpans() {
    CHECK_NEAR(PixelFont::textWidth("", 2), 0.0f, 1e-6f);
    CHECK_NEAR(PixelFont::textWidth("A", 1), 5.0f, 1e-6f);
    CHECK_NEAR(PixelFont::textWidth("AB", 2), 22.0f, 1e-6f);   // (2*6 - 1) * 2

    // 'L' is one run per row: 7 rows, doubled at scale 2.
    CHECK(PixelFont::spans("L", {0, 0}, 1).size() == 7);
    CHECK(PixelFont::spans("L", {0, 0}, 2).size() == 14);

    // Bottom row of an L at scale 1 spans the whole glyph width.
    const auto bottom = PixelFont::spans("L", {10, 20}, 1).back();
    CHECK_NEAR(bottom.first.x, 10.0f, 1e-6f);
    CHECK_NEAR(bottom.second.x, 14.0f, 1e-6f);
    CHECK_NEAR(bottom.first.y, 26.0f, 1e-6f);

    // 'H' row 0 has two separate runs (left and right stems).
    CHECK(PixelFont::spans("H", {0, 0}, 1).size() == 6 * 2 + 1);
}

void panelToggle() {
    HelpPanel panel;
    CHECK(panel.expanded());                      // open on start
    const float open = panel.height();
    CHECK(panel.headerContains({20, 20}));
    CHECK(!panel.headerContains({20, 100}));      // body is not the title bar

    panel.toggle();
    CHECK(!panel.expanded());
    CHECK(panel.height() < open);
    CHECK(panel.headerContains({20, 20}));        // the tab can reopen it
    CHECK(!panel.headerContains({HelpPanel::Left + HelpPanel::Width - 5, 20}));

    panel.toggle();
    CHECK(panel.expanded());
}

void bannerFollowsTheMission() {
    BannerData d;
    d.incidentType = 0;
    d.metresToGo = 350;
    auto t = ObjectiveBanner::text(d);
    CHECK(t.headline == "MEDICAL CALL - 350 M TO GO");
    CHECK(t.instruction == "FOLLOW THE ROUTE, STOP AT THE MARKER");

    d.nearScene = true;
    CHECK(ObjectiveBanner::text(d).instruction == "SLOW DOWN AND STOP ON THE MARKER");
    d.holding = true;
    CHECK(ObjectiveBanner::text(d).instruction == "HOLD STILL...");

    d.phase = BannerData::Phase::Arrived;
    d.responseTime = 22.6f;
    CHECK(ObjectiveBanner::text(d).headline == "ARRIVED IN 23 S");
    d.phase = BannerData::Phase::Missed;
    CHECK(ObjectiveBanner::text(d).headline == "TOO LATE");

    d.delivered = true;
    d.phase = BannerData::Phase::Arrived;
    CHECK(ObjectiveBanner::text(d).headline == "PATIENT DELIVERED IN 23 S");

    BannerData scene;
    scene.phase = BannerData::Phase::OnScene;
    scene.treatment = 0.4f;
    CHECK(ObjectiveBanner::text(scene).headline == "MEDICAL CALL - ON SCENE");
    CHECK(ObjectiveBanner::text(scene).instruction == "CREW AT WORK, STAY STOPPED 40%");

    BannerData transport;
    transport.phase = BannerData::Phase::Transport;
    transport.metresToGo = 420;
    CHECK(ObjectiveBanner::text(transport).headline == "TO HOSPITAL - 420 M TO GO");
    CHECK(ObjectiveBanner::text(transport).instruction == "PATIENT ON BOARD, FOLLOW THE ROUTE");
    transport.nearScene = true;
    CHECK(ObjectiveBanner::text(transport).instruction == "STOP IN THE HOSPITAL BAY");

    BannerData lost;
    lost.metresToGo = 300;
    lost.recalculating = true;
    CHECK(ObjectiveBanner::text(lost).instruction == "WRONG TURN - RECALCULATING ROUTE");
    lost.incidentType = static_cast<int>(IncidentType::Cardiac);
    CHECK(ObjectiveBanner::text(lost).headline == "CARDIAC ARREST - 300 M TO GO");

    BannerData fire;
    fire.incidentType = 1;
    fire.metresToGo = -4;                        // never shows a negative distance
    CHECK(ObjectiveBanner::text(fire).headline == "FIRE CALL - 0 M TO GO");
}

}

int main() {
    glyphs();
    widthsAndSpans();
    panelToggle();
    bannerFollowsTheMission();
    return check::failures();
}
