#include "Check.h"
#include "simulation/HighScores.h"
#include <cstdio>
#include <fstream>

namespace {

const char* File = "test_highscores.tmp";

void tableKeepsTheBestFive() {
    std::remove(File);
    HighScores h(File);
    CHECK(!h.load());                         // no file yet
    CHECK(h.best() == 0);
    CHECK(h.submit(0, Difficulty::Normal, 0) == -1);   // nothing to record

    CHECK(h.submit(500, Difficulty::Normal, 1) == 0);
    CHECK(h.submit(900, Difficulty::Hard, 2) == 0);    // new best
    CHECK(h.submit(700, Difficulty::Easy, 2) == 1);
    CHECK(h.submit(700, Difficulty::Normal, 3) == 2);  // tie: after the older one
    CHECK(h.submit(100, Difficulty::Easy, 0) == 4);
    CHECK(h.entries().size() == 5);
    CHECK(h.submit(50, Difficulty::Easy, 0) == -1);    // too low for a full table
    CHECK(h.submit(800, Difficulty::Hard, 2) == 1);
    CHECK(h.entries().size() == 5);
    CHECK(h.entries().back().score == 500);            // 100 dropped off
    CHECK(h.best() == 900);
}

void savedAndLoaded() {
    HighScores h(File);
    h.submit(1200, Difficulty::Hard, 4);
    h.submit(300, Difficulty::Easy, 1);
    CHECK(h.save());

    HighScores again(File);
    CHECK(again.load());
    CHECK(again.entries().size() == 2);
    CHECK(again.entries()[0].score == 1200);
    CHECK(again.entries()[0].difficulty == Difficulty::Hard);
    CHECK(again.entries()[0].calls == 4);

    // Damaged lines are skipped, order is restored.
    {
        std::ofstream out(File);
        out << "250 1 2\nnot a score\n-5 1 1\n900 7 1\n600 0 3\n";
    }
    HighScores damaged(File);
    CHECK(damaged.load());
    CHECK(damaged.entries().size() == 2);
    CHECK(damaged.entries()[0].score == 600);
    std::remove(File);
}

void difficultySettings() {
    CHECK(settingsFor(Difficulty::Easy).timeScale > 1.0f);
    CHECK(settingsFor(Difficulty::Hard).timeScale < 1.0f);
    CHECK(settingsFor(Difficulty::Hard).scoreMultiplier > settingsFor(Difficulty::Normal).scoreMultiplier);
    CHECK(settingsFor(Difficulty::Easy).trafficDensity == 0);
    CHECK(settingsFor(Difficulty::Hard).trafficDensity == 2);
}

}

int main() {
    tableKeepsTheBestFive();
    savedAndLoaded();
    difficultySettings();
    return check::failures();
}
