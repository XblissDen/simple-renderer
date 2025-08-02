#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "defines.h"
#include "rendering/renderer.h"

// Entity ID type
using EntityID = u32;
static constexpr EntityID INVALID_ENTITY = 0;

struct TransformComponent{
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f}; // euler angles
    glm::vec3 scale{1.0f};

    // Optional
    glm::mat4 worldMatrix{1.0f};
    bool isDirty = true;
};

struct RenderComponent{
    u32 modelID = 0; // Index into model array
    u32 materialID = 0; // index into material array
    bool isVisible = true;
    float lodDistance = 0.0f;
    bool castShadows = true;
};

struct HierarchyComponent {
    EntityID parent = INVALID_ENTITY;
    std::vector<EntityID> children;
};

// Component storage using Structure of Arrays
class ComponentManager{
private:
    // Entity management
    std::vector<EntityID> m_entities;
    std::vector<bool> m_activeEntities;
    EntityID m_nextEntityID = 1;

    // Component arrays - indexed by entity Id
    std::vector<TransformComponent> m_transforms;
    std::vector<RenderComponent> m_renderComponents;
    std::vector<HierarchyComponent> m_hierarchies;

    // Components masks - which components each entity has
    std::vector<u32> m_componentMasks;
public:
    // Component type flags
    enum ComponentType: u32{
        TRANSFORM = 1 << 0,
        RENDER = 1 << 1,
        HIERARCHY = 1 << 2
    };

    // Entity creation/destruction
    EntityID CreateEntity();

    void DestroyEntity(EntityID entity);

    // Component addition/removal
    void AddTransform(EntityID entity, const TransformComponent& transform = {});

    // Component addition/removal
    void AddRender(EntityID entity, const RenderComponent& render = {});

    // Component addition/removal
    void AddHierarchy(EntityID entity, const HierarchyComponent& hierarchy = {});

    // Component access
    TransformComponent* GetTransform(EntityID entity);

    RenderComponent *GetRender(EntityID entity);

    HierarchyComponent *GetHierarchy(EntityID entity);

    // Component queries - get all entities with specific components
    std::vector<EntityID> GetEntitiesWith(u32 componentMask) const;

    // Bulk data access for systems
    const std::vector<EntityID>& GetAllEntities() const { return m_entities; }
    std::vector<TransformComponent>& GetTransforms() { return m_transforms; }
    std::vector<RenderComponent>& GetRenderComponents() { return m_renderComponents; }
    const std::vector<uint32_t>& GetComponentMasks() const { return m_componentMasks; }
    const std::vector<bool>& GetActiveEntities() const { return m_activeEntities; }
};

// System base class for processing components
class System{
public:
    virtual ~System() = default;
    virtual void Update(ComponentManager& componentManager, f32 deltaTime) = 0;
};

// Transform system - updates world matrices
class TransformSystem: public System{
public:
    void Update(ComponentManager& componentManager, f32 deltaTime) override;
private:
    void UpdateWorldMatrix(TransformComponent& transform);
};

// Render system - processes renderable entities
class RenderSystem: public System{
private:
    Renderer* m_renderer;
public:
    RenderSystem(Renderer* renderer): m_renderer(renderer){}
    void Update(ComponentManager& componentManager, f32 deltaTime) override;
};