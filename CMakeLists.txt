cmake_minimum_required(VERSION 3.10)
project(SimpleRenderer VERSION 1.0)
set(OpenGL_GL_PREFERENCE "GLVND")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(GLFW_DIR ${CMAKE_SOURCE_DIR}/libs/glfw)
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/libs/glad)

set(GLFW_LIB ${CMAKE_SOURCE_DIR}/libs/glfw/lib/glfw3dll.lib)
set(ASSIMP_LIB ${CMAKE_SOURCE_DIR}/libs/assimp/lib/assimp-vc143-mtd.lib)
set(ZLIB_LIB ${CMAKE_SOURCE_DIR}/libs/assimp/lib/zlibstaticd.lib)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

find_package(OpenGL)

#include(FetchContent)
#FetchContent_Declare(
#    zlib
#    GIT_REPOSITORY https://github.com/madler/zlib.git
#    GIT_TAG v1.2.13  # or whatever is the latest version
#)
#FetchContent_MakeAvailable(zlib)

# subdirectories
#add_subdirectory( src/engine )
#add_subdirectory( src/game )
include_directories( ${CMAKE_SOURCE_DIR}/libs/glad/include )
include_directories( ${CMAKE_SOURCE_DIR}/libs/glfw/include )
include_directories( ${CMAKE_SOURCE_DIR}/libs )
include_directories( ${CMAKE_SOURCE_DIR}/include )
include_directories( ${CMAKE_SOURCE_DIR}/src )
include_directories( ${CMAKE_SOURCE_DIR}/libs/assimp )
include_directories( ${CMAKE_SOURCE_DIR}/libs/imgui )

set(SOURCES
    src/main.cpp
    #src/GameObject/GameObject.cpp
    src/ecs/component_manager.cpp
    src/ecs/scene.cpp
    #src/AssetManager/AssetManager.cpp

    libs/glad/src/gl.c
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/imgui_impl_glfw.cpp
    libs/imgui/imgui_impl_opengl3.cpp
)
set(HEADERS
    src/main.h
    src/shader.h
    src/camera.h
    src/defines.h
    #include/mesh.h
    #include/model.h
    #src/GameObject/GameObject.h
    src/ecs/component_manager.h
    src/ecs/scene.h
    src/assets/asset_manager.h
    src/rendering/gpu_resource_manager.h
    src/rendering/renderer.h
)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

#file(GLOB source_files "*.h" "*.cpp" "src/*.cpp" "include/*.h" "libs/*.h" "libs/*.c" "src/GameObject/*.cpp" "src/GameObject/*.h")
#file(GLOB_RECURSE lib_files "libs/*.h" "libs/*.c" "libs/*.cpp")
#set(ALL_SRC_FILES ${source_files} ${lib_files})

#add_executable(${PROJECT_NAME} ${ALL_SRC_FILES})

target_link_libraries(${PROJECT_NAME} ${GLFW_LIB} ${ASSIMP_LIB} ${ZLIB_LIB} OpenGL::GL)#zlibstatic

target_include_directories(SimpleRenderer PUBLIC "${PROJECT_BINARY_DIR}")
#target_include_directories(SimpleRenderer PRIVATE ${CMAKE_SOURCE_DIR}/libs/assimp)

if(MSVC)
    target_link_options(SimpleRenderer PRIVATE /ignore:4099)
endif()

add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${GLFW_DIR}/lib/glfw3.dll
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "---- Copy glfw3.dll")