cmake_minimum_required(VERSION 3.1)
project(glew)

add_library(glew INTERFACE)
target_link_libraries(glew INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glew32s.lib)
target_include_directories(glew INTERFACE include)