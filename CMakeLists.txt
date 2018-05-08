cmake_minimum_required(VERSION 3.10)
project(CSci5608_Final)
set(EXE_NAME "benchmark")

# initial number of lights
# hold up key to increase, down key to decrease
set(INITAL_LIGHTS_NUM 100)
# number of light sources will be handled per pass by deferred rendering
# as well as the maxium number of light sources will be handled by forward rendeing
set(LIGHTING_BATCH_SIZE 200)
# number of light sources in the scene along an axis
# total number of light sources will be around the square value
set(LIGHTS_OBJ_NUMBER 100) # 35 for 1296 light sources
# number of objects in the scene along an axis
# total number will be around the square value
set(SPHERES_OBJ_NUMBER 25) # 30 for 961 spheres
# perform MSAA or not
# it is more fair to compare performance of deferred and forward rendering without MSAA
set(USE_MSAA OFF)
# total number of light sources that will be processed per frame
# lights after this number will be ignored
set(MAX_LIGHT_SOURCES 100000)

set(ASSET_PATH "${CMAKE_SOURCE_DIR}/assets")


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
    set(CMAKE_CXX_FLAGS  "/W4 /O2")
else()
    set(CMAKE_CXX_FLAGS "-Wall -O3")
endif()
if (APPLE)
    set(CMAKE_MACOSX_RPATH 0)
endif()
set(CMAKE_VERBOXE_MAKEFILE TRUE)

include_directories("${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
find_package(GLM REQUIRED)
find_package(glfw3 3.2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(SOURCE_LIBRARIES
    glfw
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${FREETYPE_LIBRARIES})
if (APPLE)
    include_directories("/usr/local/include")
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVID_LIBRARY CoreVideo REQUIRED)
    set(SOURCE_LIBRARIES SOURCE_LIBRARIES
        ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVID_LIBRARY})
endif()

include_directories(${CMAKE_SOURCE_DIR}/assets)
include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
file(GLOB SHADER_FILES ${SOURCE_DIR}/shaders/*.cpp)
file(GLOB SCENE_FILES ${SOURCE_DIR}/scenes/*.cpp)
set(SOURCE_FILES
    ${SOURCE_DIR}/glfw.cpp
    ${SOURCE_DIR}/shader.cpp
    ${SOURCE_DIR}/error.cpp
    ${SOURCE_DIR}/camera.cpp
    ${SOURCE_DIR}/scene.cpp
    ${SOURCE_DIR}/app.cpp
    ${SHADER_FILES}
    ${SCENE_FILES}
    ${SOURCE_DIR}/main.cpp)
configure_file(config.h.in ${SOURCE_DIR}/config.h @ONLY)

add_executable(${EXE_NAME} ${SOURCE_FILES})
target_link_libraries(${EXE_NAME} ${SOURCE_LIBRARIES})

