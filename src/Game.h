#pragma once
#include <SFML/Graphics.hpp>
#include "render/GLRenderer.h"
#include "render/SoftwareRenderer.h"
#include <string>
#include "lines/LineRenderer.h"
#include "city/RoadNetwork.h"
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
#include "simulation/TrafficSystem.h"
#include "ui/HelpPanel.h"
#include "ui/Hud.h"
#include "ui/ObjectiveBanner.h"

// Owns every subsystem and runs the main loop.
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
    const Point startPosition_{420.0f, 175.0f};   // outside the hospital

    LineRenderer lines_{LineMode::Bresenham};
    RoadNetwork network_;
    City city_{network_};
    Environment environment_;
    Pedestrians pedestrians_{network_};
    DayNight dayNight_;
    TrafficLights trafficLights_{network_};
    Ambulance ambulance_{startPosition_};
    ClippingDemo clippingDemo_;
    Mission mission_;
    RouteGuide routeGuide_{network_};
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
    bool helpAutoHidden_{false};   // panel folds away once, on first drive
    int screenshotCount_{0};

    // Returns false when the key asks to quit.
    bool handleKey(sf::Keyboard::Key key);
    void restart();
    void step(float deltaTime, const DrivingInput& input);
    IRenderer& renderer(sf::RenderTarget& target);
    void render(IRenderer& target) const;
    void drawNightLights(IRenderer& target) const;
    HudData hudData() const;
    BannerData bannerData() const;
    void saveScreenshot(const sf::RenderWindow& window);
};
