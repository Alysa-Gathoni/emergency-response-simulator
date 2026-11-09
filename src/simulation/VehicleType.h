#pragma once

enum class VehicleType { Car, Matatu, Motorcycle };

// Per-type driving parameters. Sizes are half extents in pixels, speeds in
// pixels per second, accelerations in pixels per second squared.
struct VehicleSpec {
    VehicleType type;
    float halfLength;
    float halfWidth;
    float minCruise;
    float maxCruise;
    float acceleration;
    float braking;          // comfortable deceleration b
    float stageDwell;       // seconds stopped at a matatu stage (0 = never)
    float spawnWeight;      // relative share of the traffic mix
};

const VehicleSpec& specFor(VehicleType type);

// Maps a uniform random number u in [0, 1) to a vehicle type using the
// spawn weights (cumulative distribution: 60% cars, 25% matatus,
// 15% motorcycles).
VehicleType pickVehicleType(float u);
