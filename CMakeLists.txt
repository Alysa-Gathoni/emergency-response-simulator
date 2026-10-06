cmake_minimum_required(VERSION 3.16)
project(EmergencyResponseSimulator LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
add_executable(emergency_response_simulator
    src/main.cpp
    src/lines/LineAlgorithms.cpp
    src/city/City.cpp
)
target_include_directories(emergency_response_simulator PRIVATE src)
target_link_libraries(emergency_response_simulator PRIVATE sfml-graphics sfml-window sfml-system)
