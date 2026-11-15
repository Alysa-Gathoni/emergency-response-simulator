#pragma once
#include <string>
#include <vector>

// Difficulty of a shift. It sets the traffic density, the time limits and
// a score multiplier (harder shifts are worth more).
enum class Difficulty { Easy, Normal, Hard };
constexpr int DifficultyCount = 3;

struct DifficultySettings {
    const char* name;
    int trafficDensity;       // TrafficDensity: 0 low, 1 medium, 2 high
    float timeScale;          // multiplies every time limit
    float scoreMultiplier;    // multiplies each call score
};

const DifficultySettings& settingsFor(Difficulty difficulty);

// Best shifts, highest first, kept in a small text file next to the game:
//     <score> <difficulty 0..2> <calls completed>
// one line per entry. Lines that do not parse are skipped, so a damaged
// file never stops the game.
class HighScores {
public:
    static constexpr std::size_t MaxEntries = 5;

    struct Entry {
        int score{0};
        Difficulty difficulty{Difficulty::Normal};
        int calls{0};
    };

    explicit HighScores(std::string path = "highscores.txt");

    bool load();          // false if the file does not exist yet
    bool save() const;

    // Inserts the shift if it makes the table; returns its rank (0 = best)
    // or -1. Scores of 0 are never recorded.
    int submit(int score, Difficulty difficulty, int calls);

    const std::vector<Entry>& entries() const { return entries_; }
    int best() const { return entries_.empty() ? 0 : entries_.front().score; }

private:
    std::string path_;
    std::vector<Entry> entries_;
};
