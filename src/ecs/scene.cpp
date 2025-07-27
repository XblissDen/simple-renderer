#include "scene.h"

Scene::Scene()
{
    // Register systems
    m_systems.push_back(std::make_unique<TransformSystem>());
    m_systems.push_back(std::make_unique<RenderSystem>());
}

EntityID Scene::CreateRenderableObject(u32 modelID, u32 materialID, const glm::vec3 &position)
{
    EntityID entity = m_componentManager.CreateEntity();

    TransformComponent transform;
    transform.position = position;
    m_componentManager.AddTransform(entity, transform);

    RenderComponent render;
    render.modelID = modelID;
    render.materialID = materialID;
    m_componentManager.AddRender(entity, render);

    return entity;
}

void Scene::Update(float deltaTime)
{
    for (auto &system : m_systems)
    {
        system->Update(m_componentManager, deltaTime);
    }
}