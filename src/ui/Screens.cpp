#include "Screens.h"
#include "../simulation/Incidents.h"
#include "PixelFont.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

const Rgba Panel(14, 18, 25, 248);
const Rgba Frame(90, 100, 112);
const Rgba Title(240, 200, 80);
const Rgba Text(225, 230, 236);
const Rgba Muted(140, 150, 160);
const Rgba Good(90, 230, 120);
const Rgba Bad(250, 90, 70);
const Rgba Accent(235, 70, 70);

std::vector<Point> rect(float l, float t, float r, float b) {
    return {{l, t}, {r, t}, {r, b}, {l, b}};
}

void panel(IRenderer& target, const LineRenderer& lines, float l, float t, float r, float b) {
    PolygonFill::draw(target, rect(l, t, r, b), lines, Panel, false);
    Polygon(rect(l, t, r, b)).drawOutline(target, lines, Frame, 1.0f);
}

void centred(IRenderer& target, const std::string& text, float y, int scale, Rgba colour) {
    const float w = PixelFont::textWidth(text, scale);
    PixelFont::drawText(target, text, {Screens::Width * 0.5f - w * 0.5f, y}, scale, colour);
}

std::string mmss(float seconds) {
    const int s = std::max(0, static_cast<int>(std::lround(seconds)));
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", s / 60, s % 60);
    return buffer;
}


}

namespace Screens {

const char* menuLabel(MenuItem item) {
    switch (item) {
        case MenuItem::Start:      return "START SHIFT";
        case MenuItem::Difficulty: return "DIFFICULTY";
        case MenuItem::HighScores: return "HIGH SCORES";
        case MenuItem::Controls:   return "CONTROLS";
        case MenuItem::Quit:       return "QUIT";
    }
    return "";
}

std::string menuText(MenuItem item, Difficulty difficulty) {
    if (item == MenuItem::Difficulty)
        return std::string("DIFFICULTY: ") + settingsFor(difficulty).name;
    return menuLabel(item);
}

std::vector<std::string> highScoreLines(const HighScores& scores) {
    std::vector<std::string> out;
    int rank = 1;
    for (const auto& e : scores.entries()) {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%d.  %6d   %-6s  %d CALLS", rank++, e.score,
                      settingsFor(e.difficulty).name, e.calls);
        out.push_back(buffer);
    }
    if (out.empty()) out.push_back("NO SHIFTS RECORDED YET");
    return out;
}

void drawHighScores(IRenderer& target, const LineRenderer& lines, const HighScores& scores) {
    dim(target, lines, 160);
    panel(target, lines, 290, 170, 810, 520);
    centred(target, "HIGH SCORES", 200, 4, Title);
    float y = 270;
    for (const std::string& line : highScoreLines(scores)) {
        PixelFont::drawText(target, line, {340, y}, 2, Text);
        y += 32;
    }
    centred(target, "ENTER OR ESC: BACK", 480, 2, Muted);
}

void dim(IRenderer& target, const LineRenderer& lines, int alpha) {
    PolygonFill::draw(target, rect(0, 0, Width, Height), lines, Rgba(8, 10, 14, alpha), false);
}

void drawTitle(IRenderer& target, const LineRenderer& lines, int selected, int bestScore,
               Difficulty difficulty) {
    dim(target, lines, 150);
    panel(target, lines, 300, 110, 800, 610);

    // Red cross badge from two scan-line filled bars.
    PolygonFill::draw(target, rect(538, 134, 562, 190), lines, Accent, false);
    PolygonFill::draw(target, rect(522, 150, 578, 174), lines, Accent, false);

    centred(target, "EMERGENCY RESPONSE", 214, 4, Title);
    centred(target, "SIMULATOR", 254, 4, Title);
    centred(target, "NAIROBI DISTRICT TRAINING", 298, 2, Muted);

    for (int i = 0; i < MenuItemCount; ++i) {
        const MenuItem item = static_cast<MenuItem>(i);
        const std::string label = item == MenuItem::Difficulty && i == selected
            ? "DIFFICULTY: < " + std::string(settingsFor(difficulty).name) + " >"
            : menuText(item, difficulty);
        const float y = 344.0f + i * 40.0f;
        if (i == selected) {
            PolygonFill::draw(target, rect(400, y - 8, 700, y + 22), lines, Rgba(40, 52, 66), false);
            centred(target, "> " + label + " <", y, 2, Title);
        } else {
            centred(target, label, y, 2, Text);
        }
    }

    centred(target, "W/S OR ARROWS TO CHOOSE, A/D TO CHANGE, ENTER TO SELECT", 554, 1, Muted);
    if (bestScore > 0)
        centred(target, "BEST SHIFT SCORE " + std::to_string(bestScore), 572, 1, Muted);
}

void drawControlsFooter(IRenderer& target, const LineRenderer& lines) {
    panel(target, lines, 420, 650, 680, 690);
    centred(target, "ENTER OR ESC: BACK", 663, 2, Text);
}

void drawPause(IRenderer& target, const LineRenderer& lines) {
    dim(target, lines, 120);
    panel(target, lines, 380, 260, 720, 440);
    centred(target, "PAUSED", 288, 4, Title);
    centred(target, "P: RESUME", 344, 2, Text);
    centred(target, "R: RESTART SHIFT", 370, 2, Text);
    centred(target, "ESC: MAIN MENU", 396, 2, Text);
}

std::vector<std::string> resultLines(const MissionReport& r, const ScoreBreakdown& s, int runTotal) {
    auto signedValue = [](int v, char sign) {
        return std::string(1, sign) + std::to_string(v);
    };
    return {
        std::string("RESPONSE TIME   ") + mmss(r.elapsed) + "  (PAR " + mmss(s.parTime) + ")",
        "BASE SCORE      " + signedValue(s.base, '+'),
        "TIME BONUS      " + signedValue(s.timeBonus, '+'),
        "COLLISIONS " + std::to_string(r.collisions) + "    " + signedValue(s.collisionPenalty, '-'),
        "RED LIGHTS " + std::to_string(r.redLights) + "    " + signedValue(s.redLightPenalty, '-'),
        "WRONG TURNS " + std::to_string(r.wrongTurns) + "   " + signedValue(s.wrongTurnPenalty, '-'),
        "DIFFICULTY      " + (s.difficultyBonus < 0 ? signedValue(-s.difficultyBonus, '-')
                                                    : signedValue(s.difficultyBonus, '+')),
        "CALL SCORE      " + std::to_string(s.total),
        "SHIFT TOTAL     " + std::to_string(runTotal),
    };
}

void drawResult(IRenderer& target, const LineRenderer& lines,
                const MissionReport& r, const ScoreBreakdown& s, int runTotal) {
    dim(target, lines, 110);
    panel(target, lines, 300, 150, 800, 570);
    centred(target, r.delivered ? "PATIENT DELIVERED" : "SCENE SECURED", 176, 3, Good);
    centred(target, incidentInfo(r.incidentType).callName, 210, 2, Muted);

    float y = 252;
    for (const std::string& line : resultLines(r, s, runTotal)) {
        PixelFont::drawText(target, line, {340, y}, 2, Text);
        y += 28;
    }

    // Grade in a large letter on the right.
    const std::string grade(1, s.grade);
    const Rgba gradeColour = s.grade == 'A' || s.grade == 'B' ? Good
                           : s.grade == 'F' ? Bad : Title;
    PixelFont::drawText(target, "GRADE", {672, 316}, 2, Muted);
    PixelFont::drawText(target, grade, {682, 344}, 8, gradeColour);

    centred(target, "ENTER: NEXT CALL", 532, 2, Title);
}

void drawGameOver(IRenderer& target, const LineRenderer& lines, int callsCompleted, int runTotal,
                  int rank) {
    dim(target, lines, 150);
    panel(target, lines, 330, 220, 770, 480);
    if (rank >= 0)
        centred(target, rank == 0 ? "NEW BEST SHIFT!" : "HIGH SCORE - PLACE " + std::to_string(rank + 1),
                412, 2, Good);
    centred(target, "SHIFT OVER", 250, 4, Bad);
    centred(target, "THE PATIENT WAS NOT REACHED IN TIME", 306, 2, Text);
    centred(target, "CALLS COMPLETED  " + std::to_string(callsCompleted), 352, 2, Text);
    centred(target, "SHIFT SCORE  " + std::to_string(runTotal), 382, 2, Title);
    centred(target, "ENTER: MAIN MENU", 448, 2, Muted);
}

}
