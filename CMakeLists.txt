cmake_minimum_required(VERSION 3.16)
project(EmergencyResponseSimulator VERSION 0.7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(EmergencyResponseSimulator
    src/main.cpp
    src/Game.cpp
    src/core/Camera2D.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/polygons/Transformations.cpp
    src/polygons/Triangulation.cpp
    src/polygons/Polygon.cpp
    src/rasterization/PolygonFill.cpp
    src/rasterization/Clipping.cpp
    src/render/GLRenderer.cpp
    src/render/SoftwareRenderer.cpp
    src/shapes/CircleEllipse.cpp
    src/curves/Bezier.cpp
    src/curves/Spline.cpp
    src/curves/CatmullRom.cpp
    src/city/City.cpp
    src/city/DayNight.cpp
    src/city/Environment.cpp
    src/city/Pedestrians.cpp
    src/city/RedLightCamera.cpp
    src/city/RoadGraph.cpp
    src/city/RoadNetwork.cpp
    src/city/SignalController.cpp
    src/city/TrafficLight.cpp
    src/simulation/Ambulance.cpp
    src/simulation/ClippingDemo.cpp
    src/simulation/Collision.cpp
    src/simulation/CollisionSystem.cpp
    src/simulation/EmergencyMarker.cpp
    src/simulation/Mission.cpp
    src/simulation/Pathfinding.cpp
    src/simulation/RouteGuide.cpp
    src/simulation/SceneNode.cpp
    src/simulation/Scoring.cpp
    src/simulation/TrafficRules.cpp
    src/simulation/TrafficVehicle.cpp
    src/simulation/TrafficSystem.cpp
    src/simulation/VehicleModels.cpp
    src/simulation/VehicleType.cpp
    src/ui/HelpPanel.cpp
    src/ui/Hud.cpp
    src/ui/ObjectiveBanner.cpp
    src/ui/PixelFont.cpp
    src/ui/Screens.cpp
    src/ui/SevenSegment.cpp
)

target_include_directories(EmergencyResponseSimulator PRIVATE src)

target_link_libraries(EmergencyResponseSimulator
    sfml-graphics
    sfml-window
    sfml-system
    OpenGL::GL
)

# ---------------------------------------------------------------------------
# Unit tests: plain executables using tests/Check.h, run with `ctest`.
# Scene modules draw through the IRenderer interface only, so no test
# needs SFML or OpenGL.
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
    src/rasterization/Clipping.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/polygons/Triangulation.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_driving PRIVATE src)
add_test(NAME driving COMMAND test_driving)

add_executable(test_collision
    tests/test_collision.cpp
    src/simulation/Collision.cpp
)
target_include_directories(test_collision PRIVATE src)
add_test(NAME collision COMMAND test_collision)

add_executable(test_route
    tests/test_route.cpp
    src/simulation/Pathfinding.cpp
    src/simulation/RouteGuide.cpp
    src/city/RoadGraph.cpp
    src/city/RoadNetwork.cpp
    src/ui/PixelFont.cpp
    src/curves/Bezier.cpp
    src/curves/CatmullRom.cpp
    src/curves/Spline.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/rasterization/Clipping.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/polygons/Triangulation.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_route PRIVATE src)
add_test(NAME route COMMAND test_route)

add_executable(test_hud
    tests/test_hud.cpp
    src/ui/Hud.cpp
    src/ui/SevenSegment.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/rasterization/Clipping.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/polygons/Triangulation.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_hud PRIVATE src)
add_test(NAME hud COMMAND test_hud)

add_executable(test_help_panel
    tests/test_help_panel.cpp
    src/ui/HelpPanel.cpp
    src/ui/ObjectiveBanner.cpp
    src/ui/PixelFont.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/rasterization/Clipping.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/polygons/Triangulation.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_help_panel PRIVATE src)
add_test(NAME help_panel COMMAND test_help_panel)

add_executable(test_scoring
    tests/test_scoring.cpp
    src/simulation/Scoring.cpp
)
target_include_directories(test_scoring PRIVATE src)
add_test(NAME scoring COMMAND test_scoring)

add_executable(test_render_paths
    tests/test_render_paths.cpp
    src/simulation/VehicleModels.cpp
    src/simulation/VehicleType.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/rasterization/Clipping.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/polygons/Triangulation.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_render_paths PRIVATE src)
add_test(NAME render_paths COMMAND test_render_paths)

add_executable(test_traffic_system
    tests/test_traffic_system.cpp
    src/simulation/TrafficSystem.cpp
    src/simulation/TrafficVehicle.cpp
    src/simulation/TrafficRules.cpp
    src/simulation/VehicleType.cpp
    src/simulation/VehicleModels.cpp
    src/city/RoadNetwork.cpp
    src/city/SignalController.cpp
    src/city/TrafficLight.cpp
    src/curves/Bezier.cpp
    src/curves/CatmullRom.cpp
    src/curves/Spline.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/rasterization/Clipping.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/polygons/Triangulation.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_traffic_system PRIVATE src)
add_test(NAME traffic_system COMMAND test_traffic_system)

add_executable(test_camera
    tests/test_camera.cpp
    src/core/Camera2D.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/rasterization/Clipping.cpp
    src/rasterization/PolygonFill.cpp
    src/polygons/Triangulation.cpp
)
target_include_directories(test_camera PRIVATE src)
add_test(NAME camera COMMAND test_camera)

add_executable(test_road_graph
    tests/test_road_graph.cpp
    src/city/City.cpp
    src/city/RoadGraph.cpp
    src/city/RoadNetwork.cpp
    src/curves/Bezier.cpp
    src/curves/CatmullRom.cpp
    src/curves/Spline.cpp
    src/core/Mat3.cpp
    src/lines/LineAlgorithms.cpp
    src/lines/LineRenderer.cpp
    src/rasterization/Clipping.cpp
    src/polygons/Polygon.cpp
    src/polygons/Transformations.cpp
    src/rasterization/PolygonFill.cpp
    src/polygons/Triangulation.cpp
    src/shapes/CircleEllipse.cpp
)
target_include_directories(test_road_graph PRIVATE src)
add_test(NAME road_graph COMMAND test_road_graph)

add_executable(test_ear_clipping
    tests/test_ear_clipping.cpp
    src/polygons/Triangulation.cpp
)
target_include_directories(test_ear_clipping PRIVATE src)
add_test(NAME ear_clipping COMMAND test_ear_clipping)

add_executable(test_pathfinding
    tests/test_pathfinding.cpp
    src/simulation/Pathfinding.cpp
    src/city/RoadGraph.cpp
    src/city/RoadNetwork.cpp
    src/curves/Bezier.cpp
    src/curves/CatmullRom.cpp
    src/curves/Spline.cpp
)
target_include_directories(test_pathfinding PRIVATE src)
add_test(NAME pathfinding COMMAND test_pathfinding)
