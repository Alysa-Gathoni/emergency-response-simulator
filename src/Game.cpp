#include "Game.h"
#include "core/Color.h"
#include "rasterization/PolygonFill.h"
#include "ui/SevenSegment.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <vector>

namespace {
constexpr float MaxFrameTime = 0.25f;   // avoid a "spiral of death" after a stall
}

Game::Game() = default;

bool Game::handleKey(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Escape: return false;
        case sf::Keyboard::L:
            lines_.setMode(lines_.getMode() == LineMode::Bresenham ? LineMode::DDA
                                                                    : LineMode::Bresenham);
            break;
        case sf::Keyboard::G:
            graphicsDebug_ = !graphicsDebug_;
            lines_.setDebug(graphicsDebug_);
            break;
        case sf::Keyboard::C: clippingDebug_ = !clippingDebug_; break;
        case sf::Keyboard::B: curveDebug_ = !curveDebug_; break;
        case sf::Keyboard::T: dayNight_.nextMode(); break;
        case sf::Keyboard::P: paused_ = !paused_; break;
        case sf::Keyboard::R: restart(); break;
        default: break;
    }
    return true;
}

void Game::restart() {
    // Ambulance back at the hospital, traffic and signals back to their
    // starting state, counters cleared, first call again.
    ambulance_.resetTo(startPosition_, 0.0f);
    traffic_.reset();
    trafficLights_.reset();
    collisions_.reset();
    redLightCamera_.reset();
    mission_.restart(startPosition_);
    paused_ = false;
}

void Game::step(float dt, const DrivingInput& input) {
    const Point beforeMove = ambulance_.position();
    ambulance_.update(dt, input);
    redLightCamera_.update(beforeMove, ambulance_.position(), dt);
    trafficLights_.update(dt);
    dayNight_.update(dt);
    pedestrians_.update(dt, trafficLights_);

    // Traffic must not drive into the ambulance or people on a crossing.
    auto obstacles = pedestrians_.footprints();
    obstacles.push_back(ambulance_.footprint());
    traffic_.update(dt, trafficLights_, obstacles);

    collisions_.update(ambulance_, city_, traffic_, dt);
    mission_.update(dt, ambulance_.position(), ambulance_.speed());
    routeGuide_.update(dt, ambulance_.position(), mission_.incidentPosition(),
                       mission_.state() == Mission::State::Active);
}

void Game::drawNightLights(sf::RenderTarget& target) const {
    std::vector<DayNight::Light> lights;
    for (const auto& v : traffic_.vehicles()) {
        const Point p = v.position();
        const Point d = v.direction();
        const float front = v.halfLength();
        lights.push_back({{p.x + d.x * front, p.y + d.y * front}, d,
                          sf::Color(255, 240, 180)});
    }

    const float a = ambulance_.heading() * 3.14159265f / 180.0f;
    const Point d{std::cos(a), std::sin(a)};
    const Point p = ambulance_.position();
    lights.push_back({{p.x + d.x * 30.0f, p.y + d.y * 30.0f}, d, sf::Color(255, 250, 220)});

    for (const auto& [position, colour] : trafficLights_.litBulbs())
        lights.push_back({position, {0.0f, 0.0f}, colour});

    dayNight_.drawLights(target, lines_, lights, graphicsDebug_);
}

HudData Game::hudData() const {
    HudData h;
    h.incidentType = static_cast<int>(mission_.type());
    h.timeLeft = mission_.timeLimit() - mission_.elapsed();
    h.timeLimit = mission_.timeLimit();
    h.distance = routeGuide_.remaining();   // along the roads, not as the crow flies
    h.speed = ambulance_.speed();
    h.collisions = collisions_.collisions();
    h.redLights = redLightCamera_.violations();
    h.completed = mission_.completed();
    h.missed = mission_.missed();
    h.arrived = mission_.state() == Mission::State::Arrived;
    h.late = mission_.state() == Mission::State::Missed;
    h.bresenham = lines_.getMode() == LineMode::Bresenham;
    return h;
}

void Game::render(sf::RenderTarget& target) const {
    target.clear(Colors::Background);

    city_.draw(target, lines_, graphicsDebug_);
    environment_.draw(target, graphicsDebug_);
    trafficLights_.draw(target, lines_, graphicsDebug_);
    pedestrians_.draw(target, lines_, trafficLights_, graphicsDebug_);
    traffic_.draw(target, lines_, curveDebug_, graphicsDebug_);
    ambulance_.draw(target, lines_, graphicsDebug_);
    redLightCamera_.draw(target, lines_);
    collisions_.draw(target, lines_, graphicsDebug_);

    // Night: tint the finished scene, then draw lights on top of it.
    dayNight_.drawOverlay(target, lines_);
    drawNightLights(target);

    // Guidance stays bright at night: route and incident marker on top.
    routeGuide_.draw(target, lines_, mission_.colour(), graphicsDebug_);
    mission_.draw(target, lines_, graphicsDebug_);

    clippingDemo_.draw(target, lines_, clippingDebug_);
    hud_.draw(target, lines_, hudData());

    if (paused_) {
        const float h = 48.0f;
        const float w = SevenSegment::textWidth("PAUSE", h);
        const float x = 550.0f - w * 0.5f;
        PolygonFill::draw(target, {{x - 20, 300}, {x + w + 20, 300},
                                   {x + w + 20, 378}, {x - 20, 378}},
                          lines_, sf::Color(12, 16, 22, 200), false);
        SevenSegment::drawText(target, lines_, "PAUSE", {x, 315}, h,
                               sf::Color(240, 200, 80));
    }
}

void Game::saveScreenshot(const sf::RenderWindow& window) {
    sf::Texture texture;
    texture.create(window.getSize().x, window.getSize().y);
    texture.update(window);
    texture.copyToImage().saveToFile("screenshot-" + std::to_string(++screenshotCount_) + ".png");
}

int Game::run() {
    sf::RenderWindow window(
        sf::VideoMode(static_cast<unsigned>(RoadNetwork::WorldWidth),
                      static_cast<unsigned>(RoadNetwork::WorldHeight)),
        "Emergency Response Simulator",
        sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    sf::Clock clock;
    float accumulator = 0.0f;

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::F12) saveScreenshot(window);
                else if (!handleKey(event.key.code)) window.close();
            }
        }

        accumulator += std::min(clock.restart().asSeconds(), MaxFrameTime);

        // Driving keys only count while the window has focus (the keyboard
        // state is global, so other applications would steer otherwise).
        const DrivingInput input = window.hasFocus() ? Ambulance::readKeyboard()
                                                     : DrivingInput{};
        while (accumulator >= FixedStep) {
            if (!paused_) step(FixedStep, input);
            accumulator -= FixedStep;
        }

        render(window);
        window.display();
    }
    return 0;
}

int Game::renderScreenshot(const std::string& path, float seconds, const std::string& keys) {
    for (char k : keys) {
        const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(k)));
        if (upper >= 'A' && upper <= 'Z')
            handleKey(static_cast<sf::Keyboard::Key>(sf::Keyboard::A + (upper - 'A')));
    }

    const int steps = static_cast<int>(seconds / FixedStep);
    for (int i = 0; i < steps; ++i)
        step(FixedStep, DrivingInput{});

    sf::RenderTexture canvas;
    if (!canvas.create(static_cast<unsigned>(RoadNetwork::WorldWidth),
                       static_cast<unsigned>(RoadNetwork::WorldHeight)))
        return 1;
    render(canvas);
    canvas.display();
    return canvas.getTexture().copyToImage().saveToFile(path) ? 0 : 1;
}
