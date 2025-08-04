#include "scene.h"

Scene::Scene()
{
    // Register systems
    m_systems.push_back(std::make_unique<TransformSystem>());
    //m_systems.push_back(std::make_unique<RenderSystem>());
}

void Scene::AddSystem(std::unique_ptr<System> system)
{
    m_systems.push_back(std::move(system));
}

EntityID Scene::CreateRenderableObject(std::string name, u32 modelID, u32 materialID, const glm::vec3 &position)
{
    EntityID entity = m_componentManager.CreateEntity();

    TransformComponent transform;
    transform.position = position;
    m_componentManager.AddTransform(entity, transform);

    RenderComponent render;
    render.modelID = modelID;
    render.materialID = materialID;
    m_componentManager.AddRender(entity, render);

    m_entitiesNames[name] = entity;
    m_namesEntities[entity] = name;

    return entity;
}

void Scene::Update(float deltaTime)
{
    for (auto &system : m_systems)
    {
        system->Update(m_componentManager, deltaTime);
    }
}

EntityID Scene::GetEntityByName(std::string name)
{
    return m_entitiesNames[name];
}

std::string Scene::GetNameOfEntity(int id)
{
    return m_namesEntities[id];
}
