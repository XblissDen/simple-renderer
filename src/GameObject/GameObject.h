#pragma once
#include <vector>
#include <unordered_map>
#include <string>

#include <glm/glm.hpp>

#include "defines.h"
#include "model.h"
#include "shader.h"

// Represents a game object with transform, model, and shader
class GameObject {
public:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    Model* model;         // Assimp-loaded model
    Shader* shader;       // Shader for rendering
    bool visible = true;         // Visibility flag for culling
    u32 id = 0;          // Unique ID
    std::string Name;

    GameObject();
    void setPosition(const glm::vec3& pos);
    void setRotation(const glm::quat& rot);
    void setScale(const glm::vec3& scl);
    void setModel(Model* mod);
    void setShader(Shader* shd);
    void setVisible(bool isVisible);
    void updateModelMatrix();
    void render(const glm::mat4& viewProjMatrix);
};

// Manages all game objects
class GameObjectManager {
public:
    std::vector<GameObject> gameObjects;

    void createGameObject(std::string name);
    GameObject* getGameObject(u32 id);
    void removeGameObject(u32 id);
    void update(); // Updates all game objects (e.g., world matrices)
    void render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix); // Renders all visible game objects
    void clear();
};