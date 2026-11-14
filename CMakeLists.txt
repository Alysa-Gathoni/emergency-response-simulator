cmake_minimum_required(VERSION 3.16)
project(EmergencyResponseSimulator VERSION 0.7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(EmergencyResponseSimulator
    src/main.cpp
    src/Game.cpp
    src/core/Mat3.cpp
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
    src/city/DayNight.cpp
    src/city/Environment.cpp
    src/city/Pedestrians.cpp
    src/city/RedLightCamera.cpp
    src/city/RoadNetwork.cpp
    src/city/SignalController.cpp
    src/city/TrafficLight.cpp
    src/simulation/Ambulance.cpp
    src/simulation/ClippingDemo.cpp
    src/simulation/Collision.cpp
    src/simulation/CollisionSystem.cpp
    src/simulation/EmergencyMarker.cpp
    src/simulation/Mission.cpp
    src/simulation/RouteGuide.cpp
    src/simulation/SceneNode.cpp
    src/simulation/TrafficRules.cpp
    src/simulation/TrafficVehicle.cpp
    src/simulation/TrafficSystem.cpp
    src/simulation/VehicleModels.cpp
    src/simulation/VehicleType.cpp
    src/ui/HelpPanel.cpp
    src/ui/Hud.cpp
    src/ui/ObjectiveBanner.cpp
    src/ui/PixelFont.cpp
    src/ui/SevenSegment.cpp
)

target_include_directories(EmergencyResponseSimulator PRIVATE src)

target_link_libraries(EmergencyResponseSimulator
    sfml-graphics
    sfml-window
    sfml-system
)

# ---------------------------------------------------------------------------
# Unit tests: plain executables using tests/Check.h, run with `ctest`.
# Most link only the pure C++ modules; tests of code that draws (driving,
# route guidance, HUD) also link SFML for the shared rasterizer.
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

add_executable(test_mat3
    tests/test_mat3.cpp
    src/core/Mat3.cpp
)
target_include_directories(test_mat3 PRIVATE src)
add_test(NAME mat3 COMMAND test_mat3)

add_executable(test_driving
    tests/test_driving.cpp
    src/simulation/Ambulance.cpp
    src/simulation/SceneNode.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_driving PRIVATE src)
target_link_libraries(test_driving sfml-graphics sfml-window sfml-system)
add_test(NAME driving COMMAND test_driving)

add_executable(test_collision
    tests/test_collision.cpp
    src/simulation/Collision.cpp
)
target_include_directories(test_collision PRIVATE src)
add_test(NAME collision COMMAND test_collision)

add_executable(test_route
    tests/test_route.cpp
    src/simulation/RouteGuide.cpp
    src/city/RoadNetwork.cpp
    src/curves/Bezier.cpp
    src/curves/CatmullRom.cpp
    src/curves/Spline.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_route PRIVATE src)
target_link_libraries(test_route sfml-graphics sfml-window sfml-system)
add_test(NAME route COMMAND test_route)

add_executable(test_hud
    tests/test_hud.cpp
    src/ui/Hud.cpp
    src/ui/SevenSegment.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_hud PRIVATE src)
target_link_libraries(test_hud sfml-graphics sfml-window sfml-system)
add_test(NAME hud COMMAND test_hud)

add_executable(test_help_panel
    tests/test_help_panel.cpp
    src/ui/HelpPanel.cpp
    src/ui/ObjectiveBanner.cpp
    src/ui/PixelFont.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_help_panel PRIVATE src)
target_link_libraries(test_help_panel sfml-graphics sfml-window sfml-system)
add_test(NAME help_panel COMMAND test_help_panel)
