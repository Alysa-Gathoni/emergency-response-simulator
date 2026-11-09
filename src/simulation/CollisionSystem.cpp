#include "CollisionSystem.h"
#include "Ambulance.h"
#include "TrafficSystem.h"
#include "../city/City.h"
#include <algorithm>
#include <cmath>

namespace {

// Broad-phase radius: only boxes whose centre is this close are tested.
constexpr float NearbyDistance = 160.0f;

void outline(sf::RenderTarget& target, const LineRenderer& lines,
             const AABB& b, sf::Color c) {
    lines.draw(target, {b.minX, b.minY}, {b.maxX, b.minY}, c, 1.0f);
    lines.draw(target, {b.maxX, b.minY}, {b.maxX, b.maxY}, c, 1.0f);
    lines.draw(target, {b.maxX, b.maxY}, {b.minX, b.maxY}, c, 1.0f);
    lines.draw(target, {b.minX, b.maxY}, {b.minX, b.minY}, c, 1.0f);
}

}

void CollisionSystem::update(Ambulance& ambulance,
                             const City& city,
                             const TrafficSystem& traffic,
                             float deltaTime) {
    cooldown_ = std::max(0.0f, cooldown_ - deltaTime);
    nearby_.clear();
    hits_.clear();

    // Static obstacles first (buildings and hospital), then vehicles.
    std::vector<AABB> obstacles;
    for (const Polygon& b : city.buildings())
        obstacles.push_back(Collision::boundsOf(b.vertices()));
    obstacles.push_back(Collision::boundsOf(city.hospital().vertices()));
    for (const auto& v : traffic.vehicles())
        obstacles.push_back(Collision::boundsOf(v.footprint()));

    bool contact = false;
    // Resolving one overlap can push into another, so iterate a few times.
    for (int pass = 0; pass < 3; ++pass) {
        ambulanceBox_ = Collision::boundsOf(ambulance.footprint());
        const Point c = ambulanceBox_.centre();
        bool resolved = false;

        for (const AABB& box : obstacles) {
            const Point bc = box.centre();
            const float reach = NearbyDistance + std::max(box.width(), box.height()) * 0.5f;
            if (std::abs(bc.x - c.x) > reach || std::abs(bc.y - c.y) > reach) continue;
            if (pass == 0) nearby_.push_back(box);

            const Point push = Collision::minimumTranslation(ambulanceBox_, box);
            if (push.x == 0.0f && push.y == 0.0f) continue;

            ambulance.resolveCollision(push);
            ambulanceBox_ = Collision::boundsOf(ambulance.footprint());
            hits_.push_back(box);
            contact = true;
            resolved = true;
        }
        if (!resolved) break;
    }

    // The map edge stops the ambulance but is not counted as a crash.
    const AABB world{0, 0, 1100, 720};
    const Point fix = Collision::keepInside(ambulanceBox_, world);
    if (fix.x != 0.0f || fix.y != 0.0f) ambulance.resolveCollision(fix);

    if (contact && !wasInContact_ && cooldown_ <= 0.0f) {
        ++collisions_;
        cooldown_ = 0.5f;
    }
    wasInContact_ = contact;
}

int CollisionSystem::collisions() const { return collisions_; }
bool CollisionSystem::inContact() const { return wasInContact_; }

void CollisionSystem::reset() {
    collisions_ = 0;
    wasInContact_ = false;
    cooldown_ = 0.0f;
}

void CollisionSystem::draw(sf::RenderTarget& target, const LineRenderer& lines,
                           bool debug) const {
    if (!debug) return;
    for (const AABB& b : nearby_) outline(target, lines, b, sf::Color(240, 190, 70));
    for (const AABB& b : hits_) outline(target, lines, b, sf::Color(235, 60, 60));
    outline(target, lines, ambulanceBox_,
            wasInContact_ ? sf::Color(235, 60, 60) : sf::Color(80, 220, 170));
}
