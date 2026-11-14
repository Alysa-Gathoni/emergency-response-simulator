#pragma once
#include <string>
#include <vector>
#include "../lines/LineRenderer.h"
#include "../simulation/Scoring.h"
#include "render/IRenderer.h"

// Full-screen panels drawn over the (dimmed) city:
//
//   Title -> Start      -> Playing <-> Paused
//         -> Controls          |
//         -> Quit              +-- call reached   -> Result   -> Playing
//                              +-- time exceeded  -> GameOver -> Title
//
// Everything is drawn with our scan-line fill and 5x7 pixel font.
namespace Screens {

constexpr float Width = 1100.0f;
constexpr float Height = 720.0f;

enum class MenuItem { Start, Controls, Quit };
constexpr int MenuItemCount = 3;
const char* menuLabel(MenuItem item);

void dim(IRenderer& target, const LineRenderer& lines, int alpha);

void drawTitle(IRenderer& target, const LineRenderer& lines, int selected, int bestScore);
void drawControlsFooter(IRenderer& target, const LineRenderer& lines);
void drawPause(IRenderer& target, const LineRenderer& lines);
void drawResult(IRenderer& target, const LineRenderer& lines,
                const MissionReport& report, const ScoreBreakdown& score, int runTotal);
void drawGameOver(IRenderer& target, const LineRenderer& lines,
                  int callsCompleted, int runTotal);

// Text lines of the result panel (kept separate so they can be tested).
std::vector<std::string> resultLines(const MissionReport& report,
                                     const ScoreBreakdown& score, int runTotal);

}
