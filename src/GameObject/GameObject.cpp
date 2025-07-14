#include <GameObject.h>

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

void GameObjectManager::createGameObject(std::string name)
{
    GameObject go = GameObject();
    go.Name = name;
    go.id = gameObjects.size();

    gameObjects.push_back(go);
}

void GameObjectManager::removeGameObject(u32 id)
{
    for(u16 i = 0; i <= gameObjects.size(); i++){
        if(gameObjects[i].id == id){
            gameObjects.erase(gameObjects.begin() + i);
            break;
        }
    }
}

void GameObjectManager::update()
{
    for(u16 i = 0; i <= gameObjects.size(); i++){
        gameObjects[i].updateModelMatrix();
    }
}

void GameObjectManager::render(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix)
{
    
}
