cmake_minimum_required(VERSION 3.1)
project(ph2d)

add_library(ph2d)
target_sources(ph2d PRIVATE "src/ph2d.cpp")
target_include_directories(ph2d PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(ph2d PUBLIC glm)