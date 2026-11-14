#include <cstdlib>
#include <string>
#include "Game.h"

// Usage:
//   EmergencyResponseSimulator
//       interactive simulation
//   EmergencyResponseSimulator --screenshot <file.png> [seconds] [keys]
//       headless capture for the checkpoint evidence, e.g.
//       --screenshot night.png 20 T   (simulate 20 s at night, save a PNG)
int main(int argc, char** argv) {
    Game game;

    if (argc >= 3 && std::string(argv[1]) == "--screenshot") {
        const float seconds = argc >= 4 ? static_cast<float>(std::atof(argv[3])) : 5.0f;
        const std::string keys = argc >= 5 ? argv[4] : "";
        return game.renderScreenshot(argv[2], seconds, keys);
    }

    return game.run();
}
