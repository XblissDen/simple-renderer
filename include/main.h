#pragma once

#include <glad/gl.h>
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "defines.h"
#include "camera.h"

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

unsigned int numVerticesLoaded = 0;
unsigned int numTrianglesLoaded = 0;