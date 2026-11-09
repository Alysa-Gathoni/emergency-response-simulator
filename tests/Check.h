#pragma once
#include <cmath>
#include <iostream>

// Minimal assertion helpers for the unit tests (no framework needed).
// Each test executable returns the number of failed checks, so ctest
// reports a failure whenever any check fails.
namespace check {

inline int& failures() {
    static int count = 0;
    return count;
}

inline void report(bool ok, const char* expr, const char* file, int line) {
    if (ok) return;
    ++failures();
    std::cerr << file << ":" << line << ": check failed: " << expr << "\n";
}

inline bool near(float a, float b, float eps = 1e-3f) {
    return std::fabs(a - b) <= eps;
}

}

#define CHECK(expr) ::check::report(static_cast<bool>(expr), #expr, __FILE__, __LINE__)
#define CHECK_NEAR(a, b, eps) \
    ::check::report(::check::near((a), (b), (eps)), #a " ~= " #b, __FILE__, __LINE__)
