#pragma once

#include <glad/gl.h>
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "defines.h"
#include "camera.h"

extern int SCR_WIDTH;
extern int SCR_HEIGHT;
extern int win_w, win_h;
extern Camera camera;
extern float deltaTime;
extern float lastFrame;
extern float lastX, lastY;
extern bool firstMouse;
extern bool cursorLocked;
extern glm::vec3 lightPos;
extern unsigned int numVerticesLoaded;
extern unsigned int numTrianglesLoaded;

const glm::vec3 dirLightDirection(-0.2f, -1.0f, -0.3f);
const glm::vec3 dirLightAmbient(0.05f, 0.05f, 0.05f);
const glm::vec3 dirLightDiffuse(0.4f, 0.4f, 0.4f);
const glm::vec3 dirLightSpecular(0.5f, 0.5f, 0.5f);

const glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f, 0.2f, 2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f, 2.0f, -12.0f),
    glm::vec3(0.0f, 0.0f, -3.0f)};

const glm::vec3 pointLightColors[] = {
    glm::vec3(1.0f, 0.6f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0, 0.0),
    glm::vec3(0.2f, 0.2f, 1.0f)};
