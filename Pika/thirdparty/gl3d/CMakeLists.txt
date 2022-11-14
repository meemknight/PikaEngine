cmake_minimum_required(VERSION 3.1)
project(gl3d)

add_library(gl3d)
target_sources(gl3d PRIVATE "gl3d.cpp")
target_include_directories(gl3d PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/gl3d/")
target_link_libraries(gl3d PUBLIC glm glad stb_image stb_truetype)