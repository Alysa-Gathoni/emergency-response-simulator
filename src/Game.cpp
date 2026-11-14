#include "Game.h"
#include "core/Color.h"
#include "rasterization/PolygonFill.h"
#include "ui/PixelFont.h"
#include "ui/SevenSegment.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <vector>

namespace {
constexpr float MaxFrameTime = 0.25f;   // avoid a "spiral of death" after a stall

// Input stays in the application layer: scene modules never see SFML.
DrivingInput readDrivingKeys() {
    using K = sf::Keyboard;
    DrivingInput in;
    if (K::isKeyPressed(K::W) || K::isKeyPressed(K::Up))    in.throttle += 1.0f;
    if (K::isKeyPressed(K::S) || K::isKeyPressed(K::Down))  in.throttle -= 1.0f;
    if (K::isKeyPressed(K::A) || K::isKeyPressed(K::Left))  in.steer -= 1.0f;
    if (K::isKeyPressed(K::D) || K::isKeyPressed(K::Right)) in.steer += 1.0f;
    in.handbrake = K::isKeyPressed(K::Space);
    return in;
}
}

Game::Game() {
    camera_.setWorld(network_.width(), network_.height());
    camera_.snapTo(startPosition_);
}

bool Game::handleMenuKey(sf::Keyboard::Key key) {
    using K = sf::Keyboard;
    switch (screen_) {
        case Screen::Title:
            if (key == K::Up || key == K::W)
                menuIndex_ = (menuIndex_ + Screens::MenuItemCount - 1) % Screens::MenuItemCount;
            if (key == K::Down || key == K::S)
                menuIndex_ = (menuIndex_ + 1) % Screens::MenuItemCount;
            if (key == K::Escape) return false;
            if (key == K::Enter || key == K::Space) {
                switch (static_cast<Screens::MenuItem>(menuIndex_)) {
                    case Screens::MenuItem::Start:    startShift(); break;
                    case Screens::MenuItem::Controls: screen_ = Screen::Controls; break;
                    case Screens::MenuItem::Quit:     return false;
                }
            }
            break;
        case Screen::Controls:
            if (key == K::Escape || key == K::Enter) screen_ = Screen::Title;
            break;
        case Screen::Result:
            if (key == K::Enter || key == K::Space) screen_ = Screen::Playing;
            break;
        case Screen::GameOver:
            if (key == K::Enter || key == K::Escape) screen_ = Screen::Title;
            break;
        case Screen::Playing:
            break;
    }
    return true;
}

bool Game::handleKey(sf::Keyboard::Key key) {
    // Renderer and line algorithm can be switched on any screen.
    if (key == sf::Keyboard::O) { useOpenGL_ = !useOpenGL_; return true; }
    if (key == sf::Keyboard::L) { lines_.nextMode(); return true; }
    if (screen_ != Screen::Playing) return handleMenuKey(key);

    switch (key) {
        case sf::Keyboard::Escape:
            // First Esc pauses, a second one goes back to the main menu.
            if (paused_) { paused_ = false; screen_ = Screen::Title; }
            else paused_ = true;
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
        case sf::Keyboard::H: help_.toggle(); break;
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
    camera_.snapTo(startPosition_);
    paused_ = false;
    runTotal_ = 0;
    callStarted_ = false;
    lastMissionState_ = Mission::State::Active;
    screen_ = Screen::Playing;
}

void Game::startShift() {
    restart();
    help_.setExpanded(true);
    helpAutoHidden_ = false;
}

void Game::stepWorld(float dt) {
    // Everything that lives on without the player (also behind the menus).
    trafficLights_.update(dt);
    dayNight_.update(dt);
    pedestrians_.update(dt, trafficLights_);
    auto obstacles = pedestrians_.footprints();
    obstacles.push_back(ambulance_.footprint());
    traffic_.update(dt, trafficLights_, obstacles);
}

void Game::trackMission() {
    const Mission::State state = mission_.state();

    // A new call has just come in: remember where the counters stood and
    // how long the shortest route is (for the par time).
    if (!callStarted_ && state == Mission::State::Active) {
        callStarted_ = true;
        callStartCollisions_ = collisions_.collisions();
        callStartRedLights_ = redLightCamera_.violations();
        callRouteLength_ = routeGuide_.remaining();
    }

    if (lastMissionState_ == Mission::State::Active && state != Mission::State::Active) {
        lastReport_ = MissionReport{};
        lastReport_.incidentType = static_cast<int>(mission_.type());
        lastReport_.arrived = state == Mission::State::Arrived;
        lastReport_.elapsed = state == Mission::State::Arrived ? mission_.lastResponseTime()
                                                              : mission_.elapsed();
        lastReport_.routeLength = callRouteLength_;
        lastReport_.collisions = collisions_.collisions() - callStartCollisions_;
        lastReport_.redLights = redLightCamera_.violations() - callStartRedLights_;
        lastScore_ = Scoring::score(lastReport_);
        runTotal_ += lastScore_.total;
        bestScore_ = std::max(bestScore_, runTotal_);
        screen_ = lastReport_.arrived ? Screen::Result : Screen::GameOver;
        callStarted_ = false;
    }
    lastMissionState_ = state;
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
    trackMission();

    // Camera chases a point just ahead of the ambulance; crashes shake it.
    const float a = ambulance_.heading() * 3.14159265f / 180.0f;
    camera_.follow(ambulance_.position(),
                   {std::cos(a) * ambulance_.speed(), std::sin(a) * ambulance_.speed()}, dt);
    camera_.shake(ambulance_.impact());
}

void Game::drawNightLights(IRenderer& target, const ClipRect& visible) const {
    std::vector<DayNight::Light> lights;
    for (const auto& v : traffic_.vehicles()) {
        const Point p = v.position();
        const Point d = v.direction();
        const float front = v.halfLength();
        lights.push_back({{p.x + d.x * front, p.y + d.y * front}, d,
                          Rgba(255, 240, 180)});
    }

    const float a = ambulance_.heading() * 3.14159265f / 180.0f;
    const Point d{std::cos(a), std::sin(a)};
    const Point p = ambulance_.position();
    lights.push_back({{p.x + d.x * 30.0f, p.y + d.y * 30.0f}, d, Rgba(255, 250, 220)});

    for (const auto& [position, colour] : trafficLights_.litBulbs())
        lights.push_back({position, {0.0f, 0.0f}, colour});

    dayNight_.drawLights(target, lines_, lights, visible, graphicsDebug_);
}

BannerData Game::bannerData() const {
    BannerData b;
    switch (mission_.state()) {
        case Mission::State::Active:  b.phase = BannerData::Phase::EnRoute; break;
        case Mission::State::Arrived: b.phase = BannerData::Phase::Arrived; break;
        case Mission::State::Missed:  b.phase = BannerData::Phase::Missed; break;
    }
    b.incidentType = static_cast<int>(mission_.type());
    b.metresToGo = Hud::metres(routeGuide_.remaining());
    b.nearScene = mission_.distanceRemaining() < Mission::ArrivalRadius * 3.0f;
    b.holding = mission_.holdProgress() > 0.0f;
    b.responseTime = mission_.lastResponseTime();
    return b;
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
    h.score = runTotal_;
    // The result and game-over screens show the outcome of a call.
    h.arrived = false;
    h.late = false;
    h.lineMode = lines_.getMode() == LineMode::Bresenham ? 0
               : lines_.getMode() == LineMode::DDA ? 1 : 2;
    return h;
}

IRenderer& Game::renderer(sf::RenderTarget& target) {
    software_.setTarget(target);
    gl_.setTarget(target);
    if (useOpenGL_) return gl_;
    return software_;
}

void Game::render(IRenderer& target) {
    target.beginFrame(Colors::Background);

    // World pass: camera view matrix, and everything outside the visible
    // rectangle (or the smaller C-key window) is clipped by our algorithms
    // before rasterisation.
    const ClipRect visible = camera_.visible(-4.0f);
    const ClipRect clip = clippingDebug_ ? camera_.visible(ClippingDemo::Inset) : visible;
    target.setView(camera_.view());
    lines_.setClip(clip);

    city_.draw(target, lines_, graphicsDebug_);
    environment_.draw(target, graphicsDebug_);
    trafficLights_.draw(target, lines_, graphicsDebug_);
    pedestrians_.draw(target, lines_, trafficLights_, graphicsDebug_);
    traffic_.draw(target, lines_, curveDebug_, graphicsDebug_);
    ambulance_.draw(target, lines_, graphicsDebug_);
    redLightCamera_.draw(target, lines_);
    collisions_.draw(target, lines_, graphicsDebug_);

    // Night: tint the finished scene (screen space), then lights on top.
    target.setView(Mat3::identity());
    lines_.clearClip();
    dayNight_.drawOverlay(target, lines_, Screens::Width, Screens::Height);
    target.setView(camera_.view());
    lines_.setClip(clip);
    drawNightLights(target, clip);

    // Guidance stays bright at night: route and incident marker on top.
    routeGuide_.draw(target, lines_, mission_.colour(), graphicsDebug_);
    mission_.draw(target, lines_, graphicsDebug_);

    lines_.clearClip();
    clippingDemo_.draw(target, lines_, clippingDebug_, clip);

    // Screen pass: HUD, menus and panels.
    target.setView(Mat3::identity());

    switch (screen_) {
        case Screen::Title:
            Screens::drawTitle(target, lines_, menuIndex_, bestScore_);
            break;
        case Screen::Controls: {
            Screens::dim(target, lines_, 150);
            HelpPanel open;
            open.draw(target, lines_);
            Screens::drawControlsFooter(target, lines_);
            break;
        }
        case Screen::Playing:
        case Screen::Result:
        case Screen::GameOver: {
            hud_.draw(target, lines_, hudData());
            // Objective and controls last, so they are never covered by the scene.
            const float bannerLeft = HelpPanel::Left + help_.width() + 12.0f;
            if (screen_ == Screen::Playing) {
                banner_.draw(target, lines_, bannerData(), bannerLeft);
                help_.draw(target, lines_);
            }
            if (screen_ == Screen::Playing && paused_) Screens::drawPause(target, lines_);
            if (screen_ == Screen::Result)
                Screens::drawResult(target, lines_, lastReport_, lastScore_, runTotal_);
            if (screen_ == Screen::GameOver)
                Screens::drawGameOver(target, lines_, mission_.completed(), runTotal_);
            break;
        }
    }

    // Which backend drew this frame (for the comparison screenshots).
    PixelFont::drawText(target, target.name(), {12, 672}, 2, Rgba(140, 150, 160));
    target.endFrame();
}

void Game::saveScreenshot(const sf::RenderWindow& window) {
    sf::Texture texture;
    texture.create(window.getSize().x, window.getSize().y);
    texture.update(window);
    texture.copyToImage().saveToFile("screenshot-" + std::to_string(++screenshotCount_) + ".png");
}

int Game::run() {
    sf::RenderWindow window(
        sf::VideoMode(static_cast<unsigned>(Screens::Width),
                      static_cast<unsigned>(Screens::Height)),
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
            if (event.type == sf::Event::MouseButtonPressed
                && event.mouseButton.button == sf::Mouse::Left
                && help_.headerContains({static_cast<float>(event.mouseButton.x),
                                         static_cast<float>(event.mouseButton.y)}))
                help_.toggle();
        }

        accumulator += std::min(clock.restart().asSeconds(), MaxFrameTime);

        // Driving keys only count while the window has focus (the keyboard
        // state is global, so other applications would steer otherwise).
        const bool driving = screen_ == Screen::Playing && !paused_;
        const DrivingInput input = window.hasFocus() && driving ? readDrivingKeys()
                                                                : DrivingInput{};
        // The panel is open on start; the first press of the accelerator
        // folds it away (H brings it back), the banner keeps the goal shown.
        if (!helpAutoHidden_ && input.throttle > 0.0f) {
            help_.setExpanded(false);
            helpAutoHidden_ = true;
        }

        while (accumulator >= FixedStep) {
            if (driving) step(FixedStep, input);
            else if (screen_ != Screen::Playing) stepWorld(FixedStep);
            accumulator -= FixedStep;
        }

        render(renderer(window));
        window.display();
    }
    return 0;
}

int Game::renderScreenshot(const std::string& path, float seconds, const std::string& keys) {
    // Evidence shots show the scene with the controls panel minimised;
    // an 'H' in `keys` opens it again.
    help_.setExpanded(false);
    screen_ = Screen::Playing;
    for (char k : keys) {
        if (k == '@') { screen_ = Screen::Title; continue; }   // menu screenshot
        const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(k)));
        if (upper >= 'A' && upper <= 'Z')
            handleKey(static_cast<sf::Keyboard::Key>(sf::Keyboard::A + (upper - 'A')));
    }

    const int steps = static_cast<int>(seconds / FixedStep);
    for (int i = 0; i < steps; ++i) {
        if (screen_ == Screen::Playing) step(FixedStep, DrivingInput{});
        else stepWorld(FixedStep);
    }

    sf::RenderTexture canvas;
    if (!canvas.create(static_cast<unsigned>(Screens::Width),
                       static_cast<unsigned>(Screens::Height)))
        return 1;
    render(renderer(canvas));
    canvas.display();
    return canvas.getTexture().copyToImage().saveToFile(path) ? 0 : 1;
}
