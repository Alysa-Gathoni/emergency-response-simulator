#pragma once
#include <SFML/Graphics.hpp>
#include "render/GLRenderer.h"
#include "render/SoftwareRenderer.h"
#include <string>
#include "core/Camera2D.h"
#include "lines/LineRenderer.h"
#include "city/RoadNetwork.h"
#include "city/RoadGraph.h"
#include "city/City.h"
#include "city/TrafficLight.h"
#include "city/RedLightCamera.h"
#include "city/Environment.h"
#include "city/Pedestrians.h"
#include "city/DayNight.h"
#include "simulation/Ambulance.h"
#include "simulation/ClippingDemo.h"
#include "simulation/CollisionSystem.h"
#include "simulation/Mission.h"
#include "simulation/RouteGuide.h"
#include "simulation/Scoring.h"
#include "simulation/TrafficSystem.h"
#include "ui/HelpPanel.h"
#include "ui/Hud.h"
#include "ui/ObjectiveBanner.h"
#include "ui/Screens.h"

// Owns every subsystem and runs the main loop.
//
// Screens:  Title -> Playing (P/Esc pause) -> Result -> Playing ...
//                                          -> GameOver -> Title
// On the title screen the city keeps running behind the menu.
//
// The simulation advances in fixed steps of 1/60 s (an accumulator
// collects real frame time), so traffic, signals and driving behave the
// same at any frame rate; rendering happens once per displayed frame.
class Game {
public:
    static constexpr float FixedStep = 1.0f / 60.0f;

    Game();

    // Interactive window.
    int run();

    // Headless evidence capture: simulate `seconds` at the fixed step,
    // apply the toggle keys in `keys` (e.g. "GT", "O" for the software
    // renderer) first, render one frame off-screen and save it as a PNG.
    int renderScreenshot(const std::string& path, float seconds, const std::string& keys);

private:
    LineRenderer lines_{LineMode::Bresenham};
    RoadNetwork network_;
    RoadGraph graph_{network_};
    City city_{network_};
    const Point startPosition_ = city_.hospitalEntrance();
    Environment environment_{city_.parks()};
    Pedestrians pedestrians_{network_};
    DayNight dayNight_;
    TrafficLights trafficLights_{network_};
    Ambulance ambulance_{startPosition_};
    ClippingDemo clippingDemo_;
    Camera2D camera_{Screens::Width, Screens::Height};
    Mission mission_{graph_.incidentSpots(), startPosition_};
    RouteGuide routeGuide_{graph_};
    TrafficSystem traffic_{network_};
    RedLightCamera redLightCamera_{network_, trafficLights_};
    CollisionSystem collisions_;
    Hud hud_;
    HelpPanel help_;
    ObjectiveBanner banner_;

    // Both backends draw the same scene; O switches between them.
    SoftwareRenderer software_;
    GLRenderer gl_;
    bool useOpenGL_{true};

    bool graphicsDebug_{false};
    bool clippingDebug_{false};
    bool curveDebug_{false};
    bool paused_{false};

    enum class Screen { Title, Controls, Playing, Result, GameOver };
    Screen screen_{Screen::Title};
    int menuIndex_{0};

    // Scoring of the current shift (one run from Start to game over).
    int runTotal_{0};
    int bestScore_{0};
    MissionReport lastReport_;
    ScoreBreakdown lastScore_;
    Mission::State lastMissionState_{Mission::State::EnRoute};
    bool transportCounted_{false};
    int callStartCollisions_{0};
    int callStartRedLights_{0};
    float callRouteLength_{0.0f};
    bool callStarted_{false};
    bool helpAutoHidden_{false};   // panel folds away once, on first drive
    int screenshotCount_{0};

    // Returns false when the key asks to quit.
    bool handleKey(sf::Keyboard::Key key);
    bool handleMenuKey(sf::Keyboard::Key key);
    void restart();
    void startShift();
    void step(float deltaTime, const DrivingInput& input);
    void stepWorld(float deltaTime);
    void trackMission();
    IRenderer& renderer(sf::RenderTarget& target);
    void render(IRenderer& target);
    void drawNightLights(IRenderer& target, const ClipRect& visible) const;
    HudData hudData() const;
    BannerData bannerData() const;
    void saveScreenshot(const sf::RenderWindow& window);
};
