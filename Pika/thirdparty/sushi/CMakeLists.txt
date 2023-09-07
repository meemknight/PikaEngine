cmake_minimum_required(VERSION 3.1)
project(sushi)

add_library(sushi)

file(GLOB_RECURSE SUSHI_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

set_property(TARGET sushi PROPERTY CXX_STANDARD 17)

target_sources(sushi PRIVATE "${SUSHI_SRCS}")
target_include_directories(sushi PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(sushi PUBLIC glm glad stb_image stb_truetype gl2d)