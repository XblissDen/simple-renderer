#pragma once

#include <glad/gl.h>
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

#define Assert(condition, message, ...) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed at %s:%d: " message "\n", \
                    __FILE__, __LINE__, ##__VA_ARGS__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;
int win_w = 1280, win_h = 720;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float lastX = 640, lastY = 360;
bool firstMouse = true;

bool cursorLocked = true;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);