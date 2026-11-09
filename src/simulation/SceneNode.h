#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>
#include "../core/Mat3.h"
#include "../lines/LineRenderer.h"

// Node of a small scene graph for hierarchical models.
//
// Every node stores a LOCAL matrix relative to its parent. While drawing,
// the world matrix is accumulated down the tree:
//
//     world(node) = world(parent) * local(node)
//
// so moving the root moves the whole vehicle, and rotating a "steering"
// node turns only the wheel below it.
struct SceneNode {
    enum class Kind {
        Group,     // transform only
        Polygon,   // filled polygon (scan-line fill) with outline
        Disc,      // filled midpoint circle centred on the local origin
        Wheel      // tyre seen from above, with rolling tread lines
    };

    std::string name;
    Kind kind{Kind::Group};
    Mat3 local{Mat3::identity()};

    std::vector<Point> shape;               // Polygon: local vertices
    sf::Color fill{sf::Color::White};
    sf::Color outline{sf::Color::Transparent};
    float outlineWidth{0.0f};
    int radius{0};                          // Disc
    float wheelRadius{6.0f};                // Wheel
    float wheelWidth{5.0f};
    float spin{0.0f};                       // Wheel: rolling angle (radians)
    bool visible{true};

    std::vector<std::unique_ptr<SceneNode>> children;

    SceneNode() = default;
    SceneNode(std::string nodeName, Kind nodeKind);

    // Adds a child and returns a pointer to it for later animation.
    SceneNode* add(std::unique_ptr<SceneNode> child);
    SceneNode* find(const std::string& nodeName);

    void draw(sf::RenderTarget& target,
              const LineRenderer& lines,
              const Mat3& parentWorld,
              bool debug) const;
};

// Helpers for building models.
std::unique_ptr<SceneNode> makeGroup(const std::string& name, const Mat3& local);
std::unique_ptr<SceneNode> makeRect(const std::string& name,
                                    float left, float top, float right, float bottom,
                                    sf::Color fill,
                                    sf::Color outline = sf::Color::Transparent,
                                    float outlineWidth = 0.0f);
std::unique_ptr<SceneNode> makeDisc(const std::string& name, Point centre,
                                    int radius, sf::Color fill);
std::unique_ptr<SceneNode> makeWheel(const std::string& name,
                                     float radius, float width);
