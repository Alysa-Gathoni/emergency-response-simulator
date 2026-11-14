#include "Check.h"
#include "render/IRenderer.h"
#include "simulation/VehicleModels.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace {

// Records where every submitted pixel ends up on screen. In "hardware"
// mode it applies the current matrix itself, like OpenGL would.
class RecordingRenderer : public IRenderer {
public:
    explicit RecordingRenderer(bool hardware) : hardware_(hardware) {}

    const char* name() const override { return "TEST"; }
    void beginFrame(Rgba) override { setView(Mat3::identity()); }
    void endFrame() override {}
    bool hardwareTransforms() const override { return hardware_; }

    void drawPixels(const std::vector<Point>& pixels, Rgba, float) override {
        for (const Point& p : pixels) add(matrix_.apply(p));
    }
    void drawSpans(const std::vector<std::pair<Point, Point>>& spans, Rgba) override {
        for (const auto& s : spans) {
            add(matrix_.apply(s.first));
            add(matrix_.apply(s.second));
        }
    }
    void drawNativeLines(const std::vector<Point>&, Rgba, float) override {}
    void drawNativeTriangles(const std::vector<Point>&, Rgba) override {}

    int depth() const { return depth_; }
    float minX{std::numeric_limits<float>::max()}, maxX{-std::numeric_limits<float>::max()};
    float minY{std::numeric_limits<float>::max()}, maxY{-std::numeric_limits<float>::max()};
    int pushes{0};

protected:
    void applyMatrix(const Mat3& m) override {
        matrix_ = m;
        depth_ = (m.m[0][0] == 1.0f && m.m[0][2] == 0.0f && m.m[1][2] == 0.0f) ? 0 : 1;
        ++pushes;
    }

private:
    bool hardware_;
    Mat3 matrix_ = Mat3::identity();
    int depth_{0};
    void add(Point p) {
        minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
    }
};

VehiclePose carAt(float x, float y, float degrees) {
    VehiclePose pose{};
    pose.world = Mat3::translation(x, y) * Mat3::rotationDegrees(degrees);
    pose.type = VehicleType::Car;
    return pose;
}

void bothPathsPlaceTheCarTheSame() {
    LineRenderer lines;
    for (float angle : {0.0f, 30.0f, 90.0f, 200.0f}) {
        RecordingRenderer gpu(true), cpu(false);
        gpu.beginFrame({});
        cpu.beginFrame({});
        VehicleModels::draw(gpu, lines, carAt(400, 300, angle));
        VehicleModels::draw(cpu, lines, carAt(400, 300, angle));

        // Same footprint on screen within a couple of pixels.
        CHECK_NEAR(gpu.minX, cpu.minX, 3.0f);
        CHECK_NEAR(gpu.maxX, cpu.maxX, 3.0f);
        CHECK_NEAR(gpu.minY, cpu.minY, 3.0f);
        CHECK_NEAR(gpu.maxY, cpu.maxY, 3.0f);
    }
}

void stackIsBalanced() {
    LineRenderer lines;
    RecordingRenderer gpu(true);
    gpu.beginFrame({});
    VehicleModels::draw(gpu, lines, carAt(100, 100, 45));
    // After drawing, the renderer is back at the plain view matrix.
    const Mat3 m = gpu.current();
    CHECK_NEAR(m.m[0][2], 0.0f, 1e-4f);
    CHECK_NEAR(m.m[1][2], 0.0f, 1e-4f);
    CHECK(gpu.pushes >= 3);      // setView + push + pop
}

void pushComposesWithTheView() {
    RecordingRenderer r(true);
    r.setView(Mat3::translation(-50, -20));                 // camera
    r.pushTransform(Mat3::translation(100, 40));            // model
    const Point p = r.current().apply(Point{0, 0});
    CHECK_NEAR(p.x, 50.0f, 1e-4f);
    CHECK_NEAR(p.y, 20.0f, 1e-4f);
    r.popTransform();
    const Point q = r.current().apply(Point{0, 0});
    CHECK_NEAR(q.x, -50.0f, 1e-4f);
}

}

int main() {
    bothPathsPlaceTheCarTheSame();
    stackIsBalanced();
    pushComposesWithTheView();
    return check::failures();
}
