#include "SceneNode.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include "../shapes/CircleEllipse.h"
#include <cmath>

namespace {
constexpr float PI = 3.14159265358979323846f;
constexpr int TreadLines = 8;
}

SceneNode::SceneNode(std::string nodeName, Kind nodeKind)
    : name(std::move(nodeName)), kind(nodeKind) {}

SceneNode* SceneNode::add(std::unique_ptr<SceneNode> child) {
    children.push_back(std::move(child));
    return children.back().get();
}

SceneNode* SceneNode::find(const std::string& nodeName) {
    if (name == nodeName) return this;
    for (auto& child : children)
        if (SceneNode* hit = child->find(nodeName)) return hit;
    return nullptr;
}

void SceneNode::draw(sf::RenderTarget& target,
                     const LineRenderer& lines,
                     const Mat3& parentWorld,
                     bool debug) const {
    if (!visible) return;
    const Mat3 world = parentWorld * local;

    switch (kind) {
    case Kind::Group:
        break;

    case Kind::Polygon: {
        Polygon poly(world.apply(shape));
        PolygonFill::draw(target, poly.vertices(), lines, fill, false);
        if (outlineWidth > 0.0f)
            poly.drawOutline(target, lines, outline, outlineWidth);
        break;
    }

    case Kind::Disc:
        ShapeAlgorithms::drawPoints(
            target,
            ShapeAlgorithms::filledCircle(world.apply(Point{0, 0}), radius),
            fill, 1.0f);
        break;

    case Kind::Wheel: {
        const float r = wheelRadius;
        const float w = wheelWidth * 0.5f;
        const std::vector<Point> tyre = {{-r, -w}, {r, -w}, {r, w}, {-r, w}};
        PolygonFill::draw(target, world.apply(tyre), lines, sf::Color(22, 22, 24), false);

        // Tread ridges sit on the rim at angles spin + k * 2pi / N. Seen
        // from above, a ridge at angle a is at x = r cos(a) along the tyre
        // and is visible only on the upper half (sin(a) > 0), so as `spin`
        // grows the ridges roll across the tyre like a real wheel.
        for (int k = 0; k < TreadLines; ++k) {
            const float a = spin + k * 2.0f * PI / TreadLines;
            if (std::sin(a) <= 0.0f) continue;
            const float x = r * std::cos(a);
            lines.draw(target, world.apply(Point{x, -w}), world.apply(Point{x, w}),
                       sf::Color(90, 90, 96), 1.0f);
        }
        break;
    }
    }

    for (const auto& child : children)
        child->draw(target, lines, world, debug);

    if (debug && kind != Kind::Polygon) {
        // Local frame of this node: x axis red, y axis green.
        const Point o = world.apply(Point{0, 0});
        lines.draw(target, o, world.apply(Point{10, 0}), sf::Color(235, 80, 80), 1.0f);
        lines.draw(target, o, world.apply(Point{0, 10}), sf::Color(80, 220, 120), 1.0f);
    }
}

std::unique_ptr<SceneNode> makeGroup(const std::string& name, const Mat3& local) {
    auto node = std::make_unique<SceneNode>(name, SceneNode::Kind::Group);
    node->local = local;
    return node;
}

std::unique_ptr<SceneNode> makeRect(const std::string& name,
                                    float left, float top, float right, float bottom,
                                    sf::Color fill, sf::Color outline,
                                    float outlineWidth) {
    auto node = std::make_unique<SceneNode>(name, SceneNode::Kind::Polygon);
    node->shape = {{left, top}, {right, top}, {right, bottom}, {left, bottom}};
    node->fill = fill;
    node->outline = outline;
    node->outlineWidth = outlineWidth;
    return node;
}

std::unique_ptr<SceneNode> makeDisc(const std::string& name, Point centre,
                                    int radius, sf::Color fill) {
    auto node = std::make_unique<SceneNode>(name, SceneNode::Kind::Disc);
    node->local = Mat3::translation(centre.x, centre.y);
    node->radius = radius;
    node->fill = fill;
    return node;
}

std::unique_ptr<SceneNode> makeWheel(const std::string& name,
                                     float radius, float width) {
    auto node = std::make_unique<SceneNode>(name, SceneNode::Kind::Wheel);
    node->wheelRadius = radius;
    node->wheelWidth = width;
    return node;
}
