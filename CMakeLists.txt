cmake_minimum_required(VERSION 3.16)
project(RAY_TRACING VERSION 1.0 LANGUAGES CXX)

if (EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
  include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(glm REQUIRED)

file(GLOB SRC src/*.cpp)

add_executable(ray_tracing_program ${SRC})
target_link_libraries(ray_tracing_program PRIVATE Qt5::Widgets glm::glm)
