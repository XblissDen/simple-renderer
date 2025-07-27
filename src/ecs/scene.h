#pragma once
#include <vector>
#include <memory>

#include "defines.h"
#include "component_manager.h"

// Scene manager that owns everything
class Scene{
private:
    ComponentManager m_componentManager;
    std::vector<std::unique_ptr<System>> m_systems;

public:
    Scene();

    EntityID CreateRenderableObject(u32 modelID, u32 materialID, const glm::vec3& position = {0,0,0});
    void Update(float deltaTime);

    ComponentManager& GetComponentManager() { return m_componentManager; }
};