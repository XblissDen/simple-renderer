#include "GameObject.h"

#define STB_IMAGE_IMPLEMENTATION // TODO shouldn't be there
#include "stb_image.h"

GameObject::GameObject()
{
    Name = "Name";
}

void GameObject::setModel(Model *mod)
{
    model = mod;
}

void GameObject::setShader(Shader *shd)
{
    shader = shd;
}

void GameObject::setVisible(bool isVisible)
{
    visible = isVisible;
}

void GameObject::updateModelMatrix()
{
    modelMatrix = glm::mat4(1.0f);

    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1.0f, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1.0f));
}

void GameObject::render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix)
{
    shader->use();

    shader->setMat4("projection", projMatrix);
    shader->setMat4("view", viewMatrix);
    shader->setMat4("model", modelMatrix);

    shader->setVec3("viewPos", camera.Position);
    shader->setFloat("material.shininess", 32.0f);

    // directional light
    shader->setVec3("dirLight.direction", dirLightDirection);
    shader->setVec3("dirLight.ambient", dirLightAmbient);
    shader->setVec3("dirLight.diffuse", dirLightDiffuse);
    shader->setVec3("dirLight.specular", dirLightSpecular);

    // point light 1
    shader->setVec3("pointLights[0].position", pointLightPositions[0]);
    shader->setVec3("pointLights[0].ambient", pointLightColors[0].x * 0.1f, pointLightColors[0].y * 0.1f, pointLightColors[0].z * 0.1f);
    shader->setVec3("pointLights[0].diffuse", pointLightColors[0].x, pointLightColors[0].y, pointLightColors[0].z);
    shader->setVec3("pointLights[0].specular", pointLightColors[0].x, pointLightColors[0].y, pointLightColors[0].z);
    shader->setFloat("pointLights[0].constant", 1.0f);
    shader->setFloat("pointLights[0].linear", 0.09f);
    shader->setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    shader->setVec3("pointLights[1].position", pointLightPositions[1]);
    shader->setVec3("pointLights[1].ambient", pointLightColors[1].x * 0.1f, pointLightColors[1].y * 0.1f, pointLightColors[1].z * 0.1f);
    shader->setVec3("pointLights[1].diffuse", pointLightColors[1].x, pointLightColors[1].y, pointLightColors[1].z);
    shader->setVec3("pointLights[1].specular", pointLightColors[1].x, pointLightColors[1].y, pointLightColors[1].z);
    shader->setFloat("pointLights[1].constant", 1.0f);
    shader->setFloat("pointLights[1].linear", 0.09f);
    shader->setFloat("pointLights[1].quadratic", 0.032f);
    // point light 3
    shader->setVec3("pointLights[2].position", pointLightPositions[2]);
    shader->setVec3("pointLights[2].ambient", pointLightColors[2].x * 0.1f, pointLightColors[2].y * 0.1f, pointLightColors[2].z * 0.1f);
    shader->setVec3("pointLights[2].diffuse", pointLightColors[2].x, pointLightColors[2].y, pointLightColors[2].z);
    shader->setVec3("pointLights[2].specular", pointLightColors[2].x, pointLightColors[2].y, pointLightColors[2].z);
    shader->setFloat("pointLights[2].constant", 1.0f);
    shader->setFloat("pointLights[2].linear", 0.09f);
    shader->setFloat("pointLights[2].quadratic", 0.032f);
    // point light 4
    shader->setVec3("pointLights[3].position", pointLightPositions[3]);
    shader->setVec3("pointLights[3].ambient", pointLightColors[3].x * 0.1f, pointLightColors[3].y * 0.1f, pointLightColors[3].z * 0.1f);
    shader->setVec3("pointLights[3].diffuse", pointLightColors[3].x, pointLightColors[3].y, pointLightColors[3].z);
    shader->setVec3("pointLights[3].specular", pointLightColors[3].x, pointLightColors[3].y, pointLightColors[3].z);
    shader->setFloat("pointLights[3].constant", 1.0f);
    shader->setFloat("pointLights[3].linear", 0.09f);
    shader->setFloat("pointLights[3].quadratic", 0.032f);
    // spotLight
    shader->setVec3("spotLight.position", camera.Position);
    shader->setVec3("spotLight.direction", camera.Front);
    shader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("spotLight.constant", 1.0f);
    shader->setFloat("spotLight.linear", 0.09f);
    shader->setFloat("spotLight.quadratic", 0.032f);
    shader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    model->Draw(shader);
}

void GameObjectManager::createGameObject(std::string name)
{
    GameObject go = GameObject();
    go.Name = name;
    go.id = gameObjects.size() + 1;

    gameObjects.push_back(go);
}

void GameObjectManager::createGameObject(std::string name, Shader *shd, Model *mdl)
{
    GameObject go = GameObject();
    go.Name = name;
    go.id = gameObjects.size() + 1;
    go.shader = shd;
    go.model = mdl;

    gameObjects.push_back(go);
}

GameObject *GameObjectManager::getGameObject(u32 id)
{
    for (mm i = 0; i < gameObjects.size(); i++)
    {
        if(gameObjects[i].id == id){
            return &gameObjects[i];
        }
    }
    fprintf(stderr, "GameObjectManager: Haven't found a GameObject with ID %i", id); // TODO: maybe assert
    return nullptr;
}

GameObject *GameObjectManager::getGameObject(std::string name)
{
    for (mm i = 0; i < gameObjects.size(); i++)
    {
        if(gameObjects[i].Name.compare(name) == 0){
            return &gameObjects[i];
        }
    }
    fprintf(stderr, "GameObjectManager: Haven't found a GameObject with name %s", name.c_str()); // TODO: maybe assert
    return nullptr;
}

void GameObjectManager::removeGameObject(u32 id)
{
    for(mm i = 0; i < gameObjects.size(); i++){
        if(gameObjects[i].id == id){
            gameObjects.erase(gameObjects.begin() + i);
            break;
        }
    }
}

void GameObjectManager::update()
{
    for(mm i = 0; i < gameObjects.size(); i++){
        gameObjects[i].updateModelMatrix();
    }
}

void GameObjectManager::render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix)
{
    for (mm i = 0; i < gameObjects.size(); i++)
    {
        gameObjects[i].render(viewMatrix, projMatrix);
    }
}
