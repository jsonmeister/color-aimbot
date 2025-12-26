cmake_minimum_required(VERSION 3.16)
project(colorbot VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV REQUIRED)
find_package(hidapi CONFIG REQUIRED)

set(SOURCES
    src/main.cpp
    src/engine.cpp
    src/screen_capture.cpp
    src/dxgi_capture.cpp
    src/color_detector.cpp
    src/aim_controller.cpp
    src/arduino_mouse.cpp
    src/serial_port.cpp
    src/rawhid_controller.cpp
    src/config.cpp
)

set(HEADERS
    src/interfaces.hpp
    src/engine.hpp
    src/screen_capture.hpp
    src/dxgi_capture.hpp
    src/color_detector.hpp
    src/aim_controller.hpp
    src/arduino_mouse.hpp
    src/serial_port.hpp
    src/rawhid_controller.hpp
    src/config.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${OpenCV_LIBS}
    hidapi::hidapi
    hidapi::winapi
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601 NOMINMAX)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /O2)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O3)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
