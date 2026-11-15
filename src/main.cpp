#include <cstdlib>
#include <string>
#include "Game.h"

// Usage:
//   EmergencyResponseSimulator
//       interactive simulation
//   EmergencyResponseSimulator --screenshot <file.png> [seconds] [keys]
//       headless capture for the checkpoint evidence, e.g.
//       --screenshot night.png 20 T   (simulate 20 s at night, save a PNG)
//       --screenshot call.png 25 !    (the demo driver follows the route)
//       --screenshot menu.png 3 @     (title menu)
//       --screenshot wide.png 3 "" 1600x700   (letterboxed into another size)
int main(int argc, char** argv) {
    Game game;

    if (argc >= 3 && std::string(argv[1]) == "--screenshot") {
        const float seconds = argc >= 4 ? static_cast<float>(std::atof(argv[3])) : 5.0f;
        const std::string keys = argc >= 5 ? argv[4] : "";
        unsigned width = 0, height = 0;
        if (argc >= 6) {
            const std::string size = argv[5];
            const auto x = size.find('x');
            if (x != std::string::npos) {
                width = static_cast<unsigned>(std::atoi(size.substr(0, x).c_str()));
                height = static_cast<unsigned>(std::atoi(size.substr(x + 1).c_str()));
            }
        }
        return game.renderScreenshot(argv[2], seconds, keys, width, height);
    }

    return game.run();
}
