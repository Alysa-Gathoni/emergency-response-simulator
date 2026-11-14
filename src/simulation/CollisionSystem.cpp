#include "CollisionSystem.h"
#include "Ambulance.h"
#include "TrafficSystem.h"
#include "../city/City.h"
#include "../polygons/Triangulation.h"
#include <algorithm>
#include <cmath>

namespace {

void outline(IRenderer& target, const LineRenderer& lines,
             const AABB& b, Rgba c) {
    lines.draw(target, {b.minX, b.minY}, {b.maxX, b.minY}, c, 1.0f);
    lines.draw(target, {b.maxX, b.minY}, {b.maxX, b.maxY}, c, 1.0f);
    lines.draw(target, {b.maxX, b.maxY}, {b.minX, b.maxY}, c, 1.0f);
    lines.draw(target, {b.minX, b.maxY}, {b.minX, b.minY}, c, 1.0f);
}

void outline(IRenderer& target, const LineRenderer& lines,
             const std::vector<Point>& shape, Rgba c, float width) {
    for (std::size_t i = 0; i < shape.size(); ++i)
        lines.draw(target, shape[i], shape[(i + 1) % shape.size()], c, width);
}

bool convex(const std::vector<Point>& p) {
    // Every corner turns the same way (sign of the 2D cross product).
    float sign = 0.0f;
    for (std::size_t i = 0; i < p.size(); ++i) {
        const float c = Triangulation::cross(p[i], p[(i + 1) % p.size()], p[(i + 2) % p.size()]);
        if (std::abs(c) < 1e-4f) continue;
        if (sign == 0.0f) sign = c;
        else if ((c > 0.0f) != (sign > 0.0f)) return false;
    }
    return true;
}

}

void CollisionSystem::buildStatic(const City& city) {
    // Convex buildings are used as they are; concave ones are split into
    // their ear-clipped triangles, because SAT only works on convex shapes.
    staticPieces_.clear();
    auto add = [&](const std::vector<Point>& outlinePoints) {
        if (convex(outlinePoints)) {
            staticPieces_.push_back({outlinePoints, Collision::boundsOf(outlinePoints)});
            return;
        }
        const auto tris = Triangulation::triangles(outlinePoints);
        for (std::size_t i = 0; i + 2 < tris.size(); i += 3) {
            std::vector<Point> t = {tris[i], tris[i + 1], tris[i + 2]};
            staticPieces_.push_back({t, Collision::boundsOf(t)});
        }
    };
    for (const Polygon& b : city.buildings()) add(b.vertices());
    add(city.hospital().vertices());
    builtFor_ = &city;
}

void CollisionSystem::update(Ambulance& ambulance,
                             const City& city,
                             const TrafficSystem& traffic,
                             float deltaTime) {
    cooldown_ = std::max(0.0f, cooldown_ - deltaTime);
    if (builtFor_ != &city) buildStatic(city);

    vehicles_.clear();
    for (const auto& v : traffic.vehicles()) {
        auto shape = v.footprint();
        const AABB box = Collision::boundsOf(shape);
        vehicles_.push_back({std::move(shape), box});
    }

    candidates_.clear();
    tested_.clear();
    hits_.clear();
    broadHits_ = 0;

    bool contact = false;
    // Resolving one overlap can push into another, so iterate a few times.
    for (int pass = 0; pass < 3; ++pass) {
        ambulanceShape_ = ambulance.footprint();
        ambulanceBox_ = Collision::boundsOf(ambulanceShape_);
        bool resolved = false;

        for (const std::vector<Piece>* list : {&staticPieces_, &vehicles_})
        for (const Piece& piece : *list) {
            // Broad phase: boxes apart -> shapes apart, nothing more to do.
            if (!Collision::overlaps(ambulanceBox_, piece.box)) continue;
            if (pass == 0) {
                ++broadHits_;
                candidates_.push_back(piece.box);
                tested_.push_back(piece.shape);
            }

            // Narrow phase: exact test on the rotated shapes.
            const Collision::Contact hit = Collision::sat(ambulanceShape_, piece.shape);
            if (!hit.hit) continue;

            ambulance.resolveCollision({hit.normal.x * hit.depth, hit.normal.y * hit.depth});
            ambulanceShape_ = ambulance.footprint();
            ambulanceBox_ = Collision::boundsOf(ambulanceShape_);
            hits_.push_back(piece.shape);
            contact = true;
            resolved = true;
        }
        if (!resolved) break;
    }

    // The map edge stops the ambulance but is not counted as a crash.
    const AABB world{0, 0, city.network().width(), city.network().height()};
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

void CollisionSystem::draw(IRenderer& target, const LineRenderer& lines,
                           bool debug) const {
    if (!debug) return;
    for (const AABB& b : candidates_) outline(target, lines, b, Rgba(240, 190, 70));
    for (const auto& s : tested_) outline(target, lines, s, Rgba(255, 140, 40), 1.0f);
    for (const auto& s : hits_) outline(target, lines, s, Rgba(235, 60, 60), 2.0f);
    outline(target, lines, ambulanceShape_,
            wasInContact_ ? Rgba(235, 60, 60) : Rgba(80, 220, 170), 2.0f);
}
