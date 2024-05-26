#https://github.com/meemknight/PikaEngine


cmake_minimum_required(VERSION 3.16)

#I want to link statically to the runtime library. This will help in sharing the game with others
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization


#set glfw to NOT use dynamic runtime and not build unnecessary stuff, we have to do this before we declare projects
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) #under no circumstance we want this to happen so please cmake DON'T compile my project using shared libs


#declare projects
project(pikaGameplay)
project(pikaCore)
#project(pikaProduction)


#add third party libraries
add_subdirectory(thirdparty/glfw-3.3.2)
add_subdirectory(thirdparty/glad)
add_subdirectory(thirdparty/gl2d)
add_subdirectory(thirdparty/gl3d)
add_subdirectory(thirdparty/glui)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/stb_image)
add_subdirectory(thirdparty/stb_truetype)
add_subdirectory(thirdparty/imgui-docking)
add_subdirectory(thirdparty/safeSafe)
add_subdirectory(thirdparty/profilerLib)
add_subdirectory(thirdparty/box2d-2.4.1)
add_subdirectory(thirdparty/sushi)

#Define some macros for the project sources
file(GLOB_RECURSE PIKA_SOURCES_CORE_CONFIG			CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/coreConfig/*.cpp")
file(GLOB_RECURSE PIKA_SOURCES_CORE_EDITOR			CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaEditor/*.cpp")
file(GLOB_RECURSE PIKA_SOURCES_CORE_RUNTIME			CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaRuntime/*.cpp")
file(GLOB_RECURSE PIKA_SOURCES_CORE_STD				CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaSTD/*.cpp")
file(GLOB_RECURSE PIKA_SOURCES_GAMEPLAY				CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/gameplay/*.cpp")
file(GLOB_RECURSE PIKA_SOURCES_CORE_SHARED_RUNTIME	CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/core/sharedRuntime/*.cpp")
file(GLOB_RECURSE PIKA_SOURCES_PLUGGINS				CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/pluggins/*.cpp")

add_compile_definitions(PIKA_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/") 
add_compile_definitions(PIKA_ENGINE_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/engineResources/") 

#add_compile_definitions(PIKA_RESOURCES_PATH="./resources/")

if(MSVC)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

#https://stackoverflow.com/questions/5559357/debugheapdelete-access-violation-at-termination
#add_compile_definitions(_STATIC_CPPLIB)
#add_compile_definitions(_DISABLE_DEPRECATE_STATIC_CPPLIB)



endif()


#pikaCore ############################################
add_executable(pikaCore)

target_compile_definitions(pikaCore PUBLIC PIKA_DEVELOPMENT)

set_property(TARGET pikaCore PROPERTY CXX_STANDARD 17)
target_sources(pikaCore PRIVATE 
	"${PIKA_SOURCES_CORE_CONFIG}" "${PIKA_SOURCES_CORE_EDITOR}" "${PIKA_SOURCES_CORE_RUNTIME}"
	"${PIKA_SOURCES_CORE_STD}" "${PIKA_SOURCES_CORE_SHARED_RUNTIME}")
target_include_directories(pikaCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/coreConfig/")
target_include_directories(pikaCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaEditor/")
target_include_directories(pikaCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaRuntime/")
target_include_directories(pikaCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaSTD/")
target_include_directories(pikaCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/sharedRuntime/")
target_link_libraries(pikaCore PRIVATE glad glfw gl2d  gl3d glui
	glm stb_image stb_truetype imgui safeSave profilerLib sushi)
#################^^^^^^^^#############################


#pikaGameplay ###########################################
add_library(pikaGameplay SHARED)

target_compile_definitions(pikaGameplay PUBLIC PIKA_DEVELOPMENT)
set_property(TARGET pikaGameplay PROPERTY CXX_STANDARD 17)
target_sources(pikaGameplay PRIVATE "${PIKA_SOURCES_CORE_CONFIG}" "${PIKA_SOURCES_PLUGGINS}"
	"${PIKA_SOURCES_CORE_STD}" "${PIKA_SOURCES_GAMEPLAY}" "${PIKA_SOURCES_CORE_SHARED_RUNTIME}")
target_include_directories(pikaGameplay PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaSTD/")
target_include_directories(pikaGameplay PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/coreConfig/")
target_include_directories(pikaGameplay PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/gameplay/")
target_include_directories(pikaGameplay PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/sharedRuntime/")
target_include_directories(pikaGameplay PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/pluggins/")
target_link_libraries(pikaGameplay PRIVATE glad glfw gl2d gl3d glui glm stb_image 
	stb_truetype imgui safeSave profilerLib box2d sushi)
#################^^^^^^^^^^^^^^############################




#pikaProduction ###########################################
#add_executable(pikaProduction)
#
#target_compile_definitions(pikaProduction PUBLIC PIKA_PRODUCTION)
#set_property(TARGET pikaProduction PROPERTY CXX_STANDARD 17)
#
#target_sources(pikaProduction PRIVATE 
#	"${PIKA_SOURCES_CORE_CONFIG}" "${PIKA_SOURCES_CORE_EDITOR}" "${PIKA_SOURCES_PLUGGINS}"
#	"${PIKA_SOURCES_CORE_RUNTIME}" "${PIKA_SOURCES_CORE_STD}" "${PIKA_SOURCES_GAMEPLAY}" "${PIKA_SOURCES_CORE_SHARED_RUNTIME}")
#target_include_directories(pikaProduction PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/coreConfig/")
#target_include_directories(pikaProduction PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaEditor/")
#target_include_directories(pikaProduction PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaRuntime/")
#target_include_directories(pikaProduction PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/pikaSTD/")
#target_include_directories(pikaProduction PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/gameplay/")
#target_include_directories(pikaProduction PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/sharedRuntime/")
#target_include_directories(pikaProduction PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/pluggins/")
#
#target_link_libraries(pikaProduction PRIVATE glad glfw gl2d gl3d glui glm stb_image 
#	stb_truetype imgui safeSave profilerLib box2d sushi)
#


#################^^^^^^^^^^^^^^############################


#set exe type to be windowed
if(MSVC)
	
	#todo maybe try to bring back console in production in the future
	 #set_target_properties(pikaProduction PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
	 set_target_properties(pikaCore PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

endif()
