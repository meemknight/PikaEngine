#https://github.com/meemknight/PikaEngine


cmake_minimum_required(VERSION 3.16)

#I want to link statically to the runtime library. This will help in sharing the game with others
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")


#declare projects
project(pikaCore)



#set glfw to NOT use dynamic runtime and not build unnecessary stuff
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

#add third party libraries
add_subdirectory(thirdparty/glfw-3.3.2)
add_subdirectory(thirdparty/glad)
add_subdirectory(thirdparty/gl2d)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/stb_image)
add_subdirectory(thirdparty/stb_truetype)





#Define some macros for the project sources
file(GLOB_RECURSE PIKA_SOURCES_CORE_CONFIG	CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/coreConfig/*.cpp")
file(GLOB_RECURSE PIKA_SOURCES_CORE_EDITOR	CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaEditor/*.cpp")
file(GLOB_RECURSE PIKA_SOURCES_CORE_RUNTIME CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaRuntime/*.cpp")
file(GLOB_RECURSE PIKA_SOURCES_CORE_STD		CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaSTD/*.cpp")





add_executable(pikaCore)
set_property(TARGET pikaCore PROPERTY CXX_STANDARD 17)
target_sources(pikaCore PRIVATE 
	"${PIKA_SOURCES_CORE_CONFIG}" "${PIKA_SOURCES_CORE_EDITOR}" "${PIKA_SOURCES_CORE_RUNTIME}" "${PIKA_SOURCES_CORE_STD}")
target_include_directories(pikaCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/coreConfig/")
target_include_directories(pikaCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaEditor/")
target_include_directories(pikaCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaRuntime/")
target_include_directories(pikaCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaSTD/")
target_link_libraries(pikaCore PRIVATE glad glfw gl2d glm stb_image stb_truetype)

