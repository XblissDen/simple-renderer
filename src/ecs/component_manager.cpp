#include "component_manager.h"

EntityID ComponentManager::CreateEntity()
{
    EntityID id = m_nextEntityID++;

    // Resize arrays if needed
    if (id >= m_transforms.size())
    {
        m_transforms.resize(id + 1);
        m_renderComponents.resize(id + 1);
        m_hierarchies.resize(id + 1);
        m_componentMasks.resize(id + 1);
        m_activeEntities.resize(id + 1);
    }

    m_entities.push_back(id);
    m_activeEntities[id] = true;
    m_componentMasks[id] = 0;

    return id;
}

void ComponentManager::DestroyEntity(EntityID entity)
{
    if (entity >= m_activeEntities.size() || !m_activeEntities[entity])
        return;

    m_activeEntities[entity] = false;
    m_componentMasks[entity] = 0;

    // Remove from entity list
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);
    if (it != m_entities.end())
        m_entities.erase(it);
}

void ComponentManager::AddTransform(EntityID entity, const TransformComponent &transform)
{
    if (entity >= m_transforms.size())
        return;
    m_transforms[entity] = transform;
    m_componentMasks[entity] |= TRANSFORM;
}

void ComponentManager::AddRender(EntityID entity, const RenderComponent &render)
{
    if (entity >= m_renderComponents.size())
        return;
    m_renderComponents[entity] = render;
    m_componentMasks[entity] |= RENDER;
}

void ComponentManager::AddHierarchy(EntityID entity, const HierarchyComponent &hierarchy)
{
    if (entity >= m_hierarchies.size())
        return;
    m_hierarchies[entity] = hierarchy;
    m_componentMasks[entity] |= HIERARCHY;
}

TransformComponent *ComponentManager::GetTransform(EntityID entity)
{
    if (entity >= m_transforms.size() || !(m_componentMasks[entity] & TRANSFORM))
    {
        return nullptr;
    }
    return &m_transforms[entity];
}

RenderComponent *ComponentManager::GetRender(EntityID entity)
{
    if (entity >= m_renderComponents.size() || !(m_componentMasks[entity] & RENDER))
    {
        return nullptr;
    }
    return &m_renderComponents[entity];
}

HierarchyComponent *ComponentManager::GetHierarchy(EntityID entity)
{
    if (entity >= m_hierarchies.size() || !(m_componentMasks[entity] & HIERARCHY))
    {
        return nullptr;
    }
    return &m_hierarchies[entity];
}

std::vector<EntityID> ComponentManager::GetEntitiesWith(u32 componentMask) const
{
    std::vector<EntityID> result;

    for(EntityID entity: m_entities){
        if(m_activeEntities[entity] && (m_componentMasks[entity] & componentMask) == componentMask){
            result.push_back(entity);
        }
    }

    return result;
}

void TransformSystem::Update(ComponentManager &componentManager, f32 deltaTime)
{
    auto entities = componentManager.GetEntitiesWith(ComponentManager::TRANSFORM);

    for (EntityID entity : entities)
    {
        TransformComponent *transform = componentManager.GetTransform(entity);
        if (transform && transform->isDirty)
        {
            UpdateWorldMatrix(*transform);
            transform->isDirty = false;
        }
    }
}

void TransformSystem::UpdateWorldMatrix(TransformComponent &transform)
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), transform.position);
    glm::mat4 rotation = glm::mat4(1.0f); // Add rotation logic here
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), transform.scale);

    transform.worldMatrix = translation * rotation * scale;
}

void RenderSystem::Update(ComponentManager &componentManager, f32 deltaTime)
{
    // This would typically collect render data and submit to renderer
    auto entities = componentManager.GetEntitiesWith(
        ComponentManager::TRANSFORM | ComponentManager::RENDER);

    for (EntityID entity : entities)
    {
        TransformComponent *transform = componentManager.GetTransform(entity);
        RenderComponent *render = componentManager.GetRender(entity);

        if (render->isVisible)
        {
            // Submit to render queue with transform->worldMatrix
            // and render->modelID, render->materialID
        }
    }
}
