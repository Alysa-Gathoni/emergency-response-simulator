#include "VehicleType.h"

namespace {

// A matatu (about 60 x 24) is long and slow to accelerate and stops at
// stages; a motorcycle (about 26 x 10) is narrow, quick and brakes hard.
const VehicleSpec Specs[] = {
    {VehicleType::Car,        16.0f,  9.0f, 62.0f, 80.0f,  70.0f, 110.0f, 0.0f, 0.60f},
    {VehicleType::Matatu,     30.0f, 12.0f, 50.0f, 60.0f,  42.0f,  90.0f, 2.5f, 0.25f},
    {VehicleType::Motorcycle, 13.0f,  5.0f, 80.0f, 95.0f, 110.0f, 140.0f, 0.0f, 0.15f},
};

}

const VehicleSpec& specFor(VehicleType type) {
    for (const auto& s : Specs)
        if (s.type == type) return s;
    return Specs[0];
}

VehicleType pickVehicleType(float u) {
    float total = 0.0f;
    for (const auto& s : Specs) total += s.spawnWeight;

    float cumulative = 0.0f;
    for (const auto& s : Specs) {
        cumulative += s.spawnWeight / total;
        if (u < cumulative) return s.type;
    }
    return Specs[0].type;
}
