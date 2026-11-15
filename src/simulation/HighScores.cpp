#include "HighScores.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>

const DifficultySettings& settingsFor(Difficulty difficulty) {
    static const DifficultySettings table[DifficultyCount] = {
        {"EASY",   0, 1.30f, 0.80f},
        {"NORMAL", 1, 1.00f, 1.00f},
        {"HARD",   2, 0.80f, 1.25f},
    };
    return table[std::clamp(static_cast<int>(difficulty), 0, DifficultyCount - 1)];
}

HighScores::HighScores(std::string path) : path_(std::move(path)) {}

bool HighScores::load() {
    std::ifstream in(path_);
    if (!in) return false;
    entries_.clear();
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream fields(line);
        Entry e;
        int difficulty = 0;
        if (!(fields >> e.score >> difficulty >> e.calls)) continue;
        if (e.score <= 0 || difficulty < 0 || difficulty >= DifficultyCount || e.calls < 0) continue;
        e.difficulty = static_cast<Difficulty>(difficulty);
        entries_.push_back(e);
    }
    std::stable_sort(entries_.begin(), entries_.end(),
                     [](const Entry& a, const Entry& b) { return a.score > b.score; });
    if (entries_.size() > MaxEntries) entries_.resize(MaxEntries);
    return true;
}

bool HighScores::save() const {
    std::ofstream out(path_, std::ios::trunc);
    if (!out) return false;
    for (const Entry& e : entries_)
        out << e.score << ' ' << static_cast<int>(e.difficulty) << ' ' << e.calls << '\n';
    return static_cast<bool>(out);
}

int HighScores::submit(int score, Difficulty difficulty, int calls) {
    if (score <= 0) return -1;
    // First position whose score is lower: equal scores keep the older entry first.
    auto at = std::find_if(entries_.begin(), entries_.end(),
                           [&](const Entry& e) { return e.score < score; });
    const int rank = static_cast<int>(at - entries_.begin());
    if (rank >= static_cast<int>(MaxEntries)) return -1;
    entries_.insert(at, Entry{score, difficulty, calls});
    if (entries_.size() > MaxEntries) entries_.resize(MaxEntries);
    return rank;
}
