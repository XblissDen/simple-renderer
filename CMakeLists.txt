cmake_minimum_required(VERSION 3.10)
project(SimpleRenderer VERSION 1.0)
set(OpenGL_GL_PREFERENCE "GLVND")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(GLFW_DIR ${CMAKE_SOURCE_DIR}/libs/glfw)
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/libs/glad)

set(GLFW_LIB ${CMAKE_SOURCE_DIR}/libs/glfw/lib/glfw3dll.lib)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

#find_package(glfw3 REQUIRED)
#include_directories(${GLFW_INCLUDE_DIRS})

#find_package(GLAD REQUIRED)
#include_directories(${GLAD_INCLUDE_DIRS})

find_package(OpenGL)

# subdirectories
#add_subdirectory( src/engine )
#add_subdirectory( src/game )
include_directories( ${CMAKE_SOURCE_DIR}/libs/glad/include )
include_directories( ${CMAKE_SOURCE_DIR}/libs/glfw/include )
include_directories( ${CMAKE_SOURCE_DIR}/libs )
include_directories( ${CMAKE_SOURCE_DIR}/include )

set(SOURCES
    src/main.cpp
    libs/glad/src/gl.c
)

set(HEADERS
    include/shader.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/glfw/lib/glfw3dll.lib OpenGL::GL)

target_include_directories(SimpleRenderer PUBLIC "${PROJECT_BINARY_DIR}")

add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${GLFW_DIR}/lib/glfw3.dll
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "---- Copy glfw3.dll")