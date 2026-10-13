cmake_minimum_required(VERSION 3.16)
project(EmergencyResponseSimulator VERSION 0.5.0 LANGUAGES CXX)

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
    src/city/TrafficLight.cpp
    src/city/Environment.cpp
    src/simulation/Ambulance.cpp
    src/simulation/ClippingDemo.cpp
    src/simulation/EmergencyMarker.cpp
    src/simulation/TrafficVehicle.cpp
    src/simulation/TrafficSystem.cpp
)

target_include_directories(EmergencyResponseSimulator PRIVATE src)

target_link_libraries(EmergencyResponseSimulator
    sfml-graphics
    sfml-window
    sfml-system
)
