cmake_minimum_required(VERSION 3.16)
project(EmergencyResponseSimulator VERSION 0.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(EmergencyResponseSimulator
    src/main.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/polygons/Transformations.cpp
    src/polygons/Polygon.cpp
    src/rasterization/PolygonFill.cpp
    src/rasterization/Clipping.cpp
    src/shapes/CircleEllipse.cpp
    src/curves/Bezier.cpp
    src/curves/Spline.cpp
    src/curves/CatmullRom.cpp
    src/city/City.cpp
    src/city/Environment.cpp
    src/city/RedLightCamera.cpp
    src/city/RoadNetwork.cpp
    src/city/SignalController.cpp
    src/city/TrafficLight.cpp
    src/simulation/Ambulance.cpp
    src/simulation/ClippingDemo.cpp
    src/simulation/EmergencyMarker.cpp
    src/simulation/TrafficRules.cpp
    src/simulation/TrafficVehicle.cpp
    src/simulation/TrafficSystem.cpp
    src/simulation/VehicleType.cpp
)

target_include_directories(EmergencyResponseSimulator PRIVATE src)

target_link_libraries(EmergencyResponseSimulator
    sfml-graphics
    sfml-window
    sfml-system
)

# ---------------------------------------------------------------------------
# Unit tests: plain executables using tests/Check.h, run with `ctest`.
# They only link the pure C++ modules, so they need no window or SFML.
# ---------------------------------------------------------------------------
enable_testing()

add_executable(test_traffic_signals
    tests/test_traffic_signals.cpp
    src/city/RoadNetwork.cpp
    src/city/SignalController.cpp
    src/curves/Bezier.cpp
    src/curves/CatmullRom.cpp
    src/curves/Spline.cpp
)
target_include_directories(test_traffic_signals PRIVATE src)
add_test(NAME traffic_signals COMMAND test_traffic_signals)

add_executable(test_traffic_rules
    tests/test_traffic_rules.cpp
    src/simulation/TrafficRules.cpp
    src/simulation/VehicleType.cpp
)
target_include_directories(test_traffic_rules PRIVATE src)
add_test(NAME traffic_rules COMMAND test_traffic_rules)
