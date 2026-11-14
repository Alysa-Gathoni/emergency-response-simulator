#include "Camera2D.h"
#include <algorithm>
#include <cmath>

Camera2D::Camera2D(float viewWidth, float viewHeight)
    : viewW_(viewWidth), viewH_(viewHeight),
      worldW_(viewWidth), worldH_(viewHeight),
      centre_{viewWidth * 0.5f, viewHeight * 0.5f} {}

void Camera2D::setWorld(float width, float height) {
    worldW_ = width;
    worldH_ = height;
    centre_ = clamped(centre_);
}

Point Camera2D::clamped(Point p) const {
    // A world smaller than the window is simply centred.
    const float halfW = viewW_ * 0.5f, halfH = viewH_ * 0.5f;
    p.x = worldW_ <= viewW_ ? worldW_ * 0.5f : std::clamp(p.x, halfW, worldW_ - halfW);
    p.y = worldH_ <= viewH_ ? worldH_ * 0.5f : std::clamp(p.y, halfH, worldH_ - halfH);
    return p;
}

void Camera2D::snapTo(Point target) {
    centre_ = clamped(target);
    shake_ = 0.0f;
}

void Camera2D::follow(Point target, Point velocity, float dt) {
    // Look a little ahead in the direction of travel, capped.
    Point ahead{velocity.x * LookAhead, velocity.y * LookAhead};
    const float len = std::hypot(ahead.x, ahead.y);
    if (len > MaxLookAhead) {
        ahead.x *= MaxLookAhead / len;
        ahead.y *= MaxLookAhead / len;
    }
    const Point desired = clamped({target.x + ahead.x, target.y + ahead.y});

    const float k = 1.0f - std::exp(-Stiffness * dt);
    centre_.x += (desired.x - centre_.x) * k;
    centre_.y += (desired.y - centre_.y) * k;
    centre_ = clamped(centre_);

    time_ += dt;
    shake_ = std::max(0.0f, shake_ - 2.5f * dt);
}

void Camera2D::shake(float strength) {
    shake_ = std::max(shake_, std::clamp(strength, 0.0f, 1.0f));
}

Point Camera2D::shakeOffset() const {
    const float a = 7.0f * shake_;
    return {a * std::sin(time_ * 53.0f), a * std::cos(time_ * 47.0f)};
}

Point Camera2D::centre() const { return centre_; }

Mat3 Camera2D::view() const {
    const Point s = shakeOffset();
    return Mat3::translation(viewW_ * 0.5f, viewH_ * 0.5f) *
           Mat3::translation(-(centre_.x + s.x), -(centre_.y + s.y));
}

ClipRect Camera2D::visible(float inset) const {
    return {centre_.x - viewW_ * 0.5f + inset, centre_.y - viewH_ * 0.5f + inset,
            centre_.x + viewW_ * 0.5f - inset, centre_.y + viewH_ * 0.5f - inset};
}

Point Camera2D::toScreen(Point world) const { return view().apply(world); }
Point Camera2D::toWorld(Point screen) const { return view().inverse().apply(screen); }
