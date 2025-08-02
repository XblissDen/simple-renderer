#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include "defines.h"
#include "component_manager.h"

// Scene manager that owns everything
class Scene{
private:
    ComponentManager m_componentManager;
    std::vector<std::unique_ptr<System>> m_systems;
    std::unordered_map<std::string, EntityID> m_entitiesNames;

public:
    Scene();

    void AddSystem(std::unique_ptr<System> system);

    EntityID CreateRenderableObject(std::string name, u32 modelID, u32 materialID, const glm::vec3& position = {0,0,0});
    void Update(float deltaTime);

    EntityID GetEntityByName(std::string name);
    ComponentManager& GetComponentManager() { return m_componentManager; }
};