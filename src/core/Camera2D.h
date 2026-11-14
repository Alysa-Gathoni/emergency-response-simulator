#pragma once
#include "Mat3.h"
#include "Point.h"
#include "../rasterization/Clipping.h"

// 2D camera that follows the ambulance through a world bigger than the
// window.
//
//   view = T(viewWidth / 2, viewHeight / 2) * T(-(centre + shake))
//
// maps the camera centre to the middle of the window; the renderer loads
// it as the view matrix (glLoadMatrixf in OpenGL). The centre chases a
// point slightly ahead of the ambulance with exponential smoothing,
//
//   centre += (desired - centre) * (1 - e^(-k * dt)),
//
// which is frame-rate independent, and is clamped so the window never
// shows anything outside the world.
class Camera2D {
public:
    Camera2D(float viewWidth, float viewHeight);

    void setWorld(float width, float height);
    void snapTo(Point target);

    // `velocity` (px/s) moves the look-ahead point in front of the car.
    void follow(Point target, Point velocity, float deltaTime);

    // Short shake after a crash (0..1, fades by itself).
    void shake(float strength);

    Point centre() const;
    Mat3 view() const;

    // World-space rectangle visible in the window, optionally shrunk.
    ClipRect visible(float inset = 0.0f) const;

    Point toScreen(Point world) const;
    Point toWorld(Point screen) const;

    static constexpr float Stiffness = 4.0f;      // 1/s, how quickly it catches up
    static constexpr float LookAhead = 0.45f;     // seconds of travel shown ahead
    static constexpr float MaxLookAhead = 140.0f; // px

private:
    float viewW_, viewH_;
    float worldW_, worldH_;
    Point centre_;
    float shake_{0.0f};
    float time_{0.0f};

    Point clamped(Point p) const;
    Point shakeOffset() const;
};
