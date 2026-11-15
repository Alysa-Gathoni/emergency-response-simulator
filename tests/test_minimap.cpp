#include "Check.h"
#include "city/City.h"
#include "city/RoadNetwork.h"
#include "ui/Minimap.h"
#include <algorithm>
#include <limits>

namespace {

// Records the extent of everything drawn (no window needed).
class BoundsRenderer : public IRenderer {
public:
    float minX = std::numeric_limits<float>::max(), minY = minX;
    float maxX = std::numeric_limits<float>::lowest(), maxY = maxX;
    int calls = 0;

    const char* name() const override { return "TEST"; }
    void beginFrame(Rgba) override {}
    void endFrame() override {}
    bool hardwareTransforms() const override { return false; }
    void drawPixels(const std::vector<Point>& pixels, Rgba, float size) override {
        for (const Point& p : pixels) { grow(p); grow({p.x + size, p.y + size}); }
        ++calls;
    }
    void drawSpans(const std::vector<std::pair<Point, Point>>& spans, Rgba) override {
        for (const auto& s : spans) { grow(s.first); grow({s.second.x + 1.0f, s.second.y + 1.0f}); }
        ++calls;
    }
    void drawNativeLines(const std::vector<Point>& p, Rgba, float) override { for (auto q : p) grow(q); }
    void drawNativeTriangles(const std::vector<Point>& p, Rgba) override { for (auto q : p) grow(q); }

protected:
    void applyMatrix(const Mat3&) override {}

private:
    void grow(Point p) {
        minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
    }
};

void windowToViewportKeepsAspect() {
    // 2000 x 1000 world into a 200 x 200 viewport: s = min(0.1, 0.2) = 0.1,
    // map 200 x 100 centred vertically.
    const Mat3 m = Minimap::windowToViewport({0, 0, 2000, 1000}, {100, 100, 300, 300});
    const Point tl = m.apply(Point{0, 0});
    const Point br = m.apply(Point{2000, 1000});
    const Point c = m.apply(Point{1000, 500});
    CHECK_NEAR(tl.x, 100.0f, 1e-3f);
    CHECK_NEAR(tl.y, 150.0f, 1e-3f);
    CHECK_NEAR(br.x, 300.0f, 1e-3f);
    CHECK_NEAR(br.y, 250.0f, 1e-3f);
    CHECK_NEAR(c.x, 200.0f, 1e-3f);          // centre to centre
    CHECK_NEAR(c.y, 200.0f, 1e-3f);
    // Uniform scale: a 100 x 100 square stays square.
    const Point a = m.apply(Point{500, 500}), b = m.apply(Point{600, 600});
    CHECK_NEAR(b.x - a.x, b.y - a.y, 1e-4f);
}

void drawingStaysInsideTheViewport() {
    RoadNetwork network;
    City city(network);
    const ClipRect viewport{862, 552, 1088, 708};
    Minimap map(city, viewport);

    // The whole district fits inside the viewport.
    const Point far = map.transform().apply(Point{network.width(), network.height()});
    CHECK(far.x <= viewport.right && far.y <= viewport.bottom);

    BoundsRenderer r;
    LineRenderer lines;
    Minimap::Frame f;
    f.ambulance = city.hospitalEntrance();
    f.route = {f.ambulance, {f.ambulance.x + 900, f.ambulance.y}, {-500, 3000}};   // leaves the map
    f.routeColour = Rgba(235, 70, 70);
    f.showTarget = true;
    f.target = {f.ambulance.x + 900, f.ambulance.y};
    f.traffic = {{100, 100}, {-50, 50}, {5000, 5000}};
    f.cameraView = {0, 0, 1100, 720};
    map.draw(r, lines, f);

    CHECK(r.calls > 10);
    CHECK(r.minX >= viewport.left - 0.01f);
    CHECK(r.minY >= viewport.top - 0.01f);
    CHECK(r.maxX <= viewport.right + 2.01f);   // + one line pixel at the edge
    CHECK(r.maxY <= viewport.bottom + 2.01f);
    CHECK(!lines.clip());                      // clip window restored

    // Hidden: nothing drawn.
    BoundsRenderer none;
    map.toggle();
    map.draw(none, lines, f);
    CHECK(none.calls == 0);
}

}

int main() {
    windowToViewportKeepsAspect();
    drawingStaysInsideTheViewport();
    return check::failures();
}
