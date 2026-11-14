#include "Check.h"
#include "core/Camera2D.h"
#include "lines/LineRenderer.h"
#include "render/IRenderer.h"
#include "rasterization/PolygonFill.h"

namespace {

class CountingRenderer : public IRenderer {
public:
    const char* name() const override { return "COUNT"; }
    void beginFrame(Rgba) override {}
    void endFrame() override {}
    bool hardwareTransforms() const override { return false; }
    void drawPixels(const std::vector<Point>& p, Rgba, float) override { pixels += p.size(); }
    void drawSpans(const std::vector<std::pair<Point, Point>>& s, Rgba) override { spans += s.size(); }
    void drawNativeLines(const std::vector<Point>&, Rgba, float) override {}
    void drawNativeTriangles(const std::vector<Point>&, Rgba) override {}
    std::size_t pixels{0}, spans{0};
protected:
    void applyMatrix(const Mat3&) override {}
};

void viewPutsTheCentreInTheMiddle() {
    Camera2D cam(1100, 720);
    cam.setWorld(3000, 2000);
    cam.snapTo({1500, 1000});
    const Point p = cam.toScreen({1500, 1000});
    CHECK_NEAR(p.x, 550.0f, 1e-3f);
    CHECK_NEAR(p.y, 360.0f, 1e-3f);
    const Point back = cam.toWorld({0, 0});
    CHECK_NEAR(back.x, 950.0f, 1e-3f);
    CHECK_NEAR(back.y, 640.0f, 1e-3f);
}

void clampedToTheWorld() {
    Camera2D cam(1100, 720);
    cam.setWorld(3000, 2000);
    cam.snapTo({10, 10});                       // near the top-left corner
    const ClipRect v = cam.visible();
    CHECK_NEAR(v.left, 0.0f, 1e-3f);
    CHECK_NEAR(v.top, 0.0f, 1e-3f);
    cam.snapTo({2990, 1990});
    CHECK_NEAR(cam.visible().right, 3000.0f, 1e-3f);
    CHECK_NEAR(cam.visible().bottom, 2000.0f, 1e-3f);

    Camera2D small(1100, 720);                  // world smaller than the window
    small.setWorld(800, 600);
    small.snapTo({0, 0});
    CHECK_NEAR(small.centre().x, 400.0f, 1e-3f);
}

void followConvergesSmoothly() {
    Camera2D cam(1100, 720);
    cam.setWorld(4000, 4000);
    cam.snapTo({1000, 1000});
    const float dt = 1.0f / 60.0f;
    cam.follow({1400, 1000}, {0, 0}, dt);
    const float firstStep = cam.centre().x - 1000.0f;
    CHECK(firstStep > 0.0f && firstStep < 40.0f);          // eases, does not jump
    for (int i = 0; i < 180; ++i) cam.follow({1400, 1000}, {0, 0}, dt);
    CHECK_NEAR(cam.centre().x, 1400.0f, 1.0f);              // arrives within 3 s

    // Driving right at speed: the camera looks ahead of the car.
    for (int i = 0; i < 300; ++i) cam.follow({2000, 1000}, {140, 0}, dt);
    CHECK(cam.centre().x > 2000.0f + 40.0f);
}

void cullingSkipsWhatIsOffScreen() {
    LineRenderer lines;
    CountingRenderer r;
    lines.setClip({0, 0, 100, 100});

    lines.draw(r, {200, 200}, {300, 250}, Rgba::White, 1.0f);   // completely outside
    CHECK(r.pixels == 0);

    lines.draw(r, {50, 50}, {250, 50}, Rgba::White, 1.0f);      // clipped at x = 100
    CHECK(r.pixels == 51);

    PolygonFill::draw(r, {{300, 300}, {400, 300}, {400, 400}}, lines, Rgba::White, false);
    CHECK(r.spans == 0);                                           // culled
    PolygonFill::draw(r, {{50, 50}, {500, 50}, {500, 500}, {50, 500}}, lines, Rgba::White, false);
    CHECK(r.spans > 0 && r.spans <= 52);                           // only rows 50..100

    // Shapes in local coordinates (hardware transform stack) are tested
    // in world space: culled when the model puts them off screen.
    r.spans = 0;
    r.pushTransform(Mat3::translation(1000, 1000));
    PolygonFill::draw(r, {{0, 0}, {20, 0}, {20, 20}}, lines, Rgba::White, false);
    CHECK(r.spans == 0);
    r.popTransform();
    r.pushTransform(Mat3::translation(40, 40));
    PolygonFill::draw(r, {{0, 0}, {20, 0}, {20, 20}}, lines, Rgba::White, false);
    CHECK(r.spans > 0);
    r.popTransform();

    lines.clearClip();
    r.pixels = 0;
    lines.draw(r, {200, 200}, {300, 200}, Rgba::White, 1.0f);
    CHECK(r.pixels == 101);
}

}

int main() {
    viewPutsTheCentreInTheMiddle();
    clampedToTheWorld();
    followConvergesSmoothly();
    cullingSkipsWhatIsOffScreen();
    return check::failures();
}
