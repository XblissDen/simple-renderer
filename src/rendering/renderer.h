#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "assets/asset_manager.h"  
#include "rendering/gpu_resource_manager.h"  
#include "shader.h"
#include "defines.h"

class Renderer{
private:
    AssetManager* m_assetManager;
    GPUResourceManager* m_gpuResourceManager;

    // Render commands for current frame
    std::vector<RenderCommand> m_renderCommands;
    std::vector<RenderBatch> m_renderBatches;

    // Camera data
    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projectionMatrix{1.0f};
    glm::vec3 m_cameraPosition{0.0f};

    // Shaders
    std::unique_ptr<Shader> m_defaultShader;
    Shader* m_currentShader;

    // Statistics
    u32 m_drawCalls = 0;
    u32 m_trianglesRendered = 0;

public:
    Renderer(AssetManager* assetManager, GPUResourceManager* gpuResourceManager)
    : m_assetManager(assetManager), m_gpuResourceManager(gpuResourceManager){
        // Load default shader
        m_defaultShader = std::make_unique<Shader>("modelShader.vert", "modelShader.frag");
    }

    // Camera setup
    void SetCamera(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& position) {
        m_viewMatrix = view;
        m_projectionMatrix = projection;
        m_cameraPosition = position;
    }

    // Add render command for this frame
    void SubmitRenderCommand(const RenderCommand& command) {
        m_renderCommands.push_back(command);
    }

    // Render all submitted commands
    void RenderFrame(){
        // Clear statistics
        m_drawCalls = 0;
        m_trianglesRendered = 0;

        if(m_renderCommands.empty()) return;

        // Calculate distance to camera for sorting
        CalculateDistances();
        
        // Build render batches
        BuildRenderBatches();
        
        // Sort batches
        SortRenderBatches();
        
        // Set up global rendering state
        SetupGlobalState();

        // Render all batches
        for (const auto& batch : m_renderBatches) {
            RenderBatch(batch);
        }

        // Cleanup for next frame
        m_renderCommands.clear();
        m_renderBatches.clear();
    }

    // Statistics
    u32 GetDrawCalls() const { return m_drawCalls; }
    u32 GetTrianglesRendered() const { return m_trianglesRendered; }

private:
    void CalculateDistances(){
        for(auto& command: m_renderCommands){
            glm::vec3 objectPosition = glm::vec3(command.worldMatrix[3]);
            command.distanceToCamera = glm::distance(m_cameraPosition, objectPosition);
        }
    }

    void BuildRenderBatches(){
        // Group commands by material
        std::unordered_map<MaterialID, std::vector<RenderCommand*>> materialGroups;
        
        for (auto& command : m_renderCommands) {
            materialGroups[command.materialID].push_back(&command);
        }

        // Create batches
        m_renderBatches.clear();
        m_renderBatches.reserve(materialGroups.size());

        for (auto& [materialID, commands] : materialGroups) {
            RenderBatch batch;
            batch.materialID = materialID;
            batch.commands = std::move(commands);
            
            // Check if material is transparent
            const Material* material = m_assetManager->GetMaterial(materialID);
            batch.isTransparent = material && IsTransparent(*material);
            
            m_renderBatches.push_back(std::move(batch));
        }
    }

    void SortRenderBatches() {
        // Sort batches: opaque first, then transparent
        std::sort(m_renderBatches.begin(), m_renderBatches.end(),
                  [](const RenderBatch& a, const RenderBatch& b) {
                      if (a.isTransparent != b.isTransparent) {
                          return !a.isTransparent;  // Opaque first
                      }
                      return a.materialID < b.materialID;  // Group by material
                  });
        
        // Sort commands within each batch
        for (auto& batch : m_renderBatches) {
            if (batch.isTransparent) {
                // Transparent: back to front
                std::sort(batch.commands.begin(), batch.commands.end(),
                          [](const RenderCommand* a, const RenderCommand* b) {
                              return a->distanceToCamera > b->distanceToCamera;
                          });
            } else {
                // Opaque: front to back (early Z rejection)
                std::sort(batch.commands.begin(), batch.commands.end(),
                          [](const RenderCommand* a, const RenderCommand* b) {
                              return a->distanceToCamera < b->distanceToCamera;
                          });
            }
        }
    }

    void SetupGlobalState() {
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        // Enable face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        
        // Set default shader
        m_currentShader = m_defaultShader.get();
        m_currentShader->use();
        
        // Set camera matrices
        m_currentShader->setMat4("view", m_viewMatrix);
        m_currentShader->setMat4("projection", m_projectionMatrix);
        m_currentShader->setVec3("viewPos", m_cameraPosition);
    }

    void DrawBatch(const RenderBatch& batch) {
        // Bind material
        BindMaterial(batch.materialID);
        
        // Render all commands in this batch
        for (const RenderCommand* command : batch.commands) {
            RenderCommand(*command);
        }
    }

    void BindMaterial(MaterialID materialID) {
        const Material* material = m_assetManager->GetMaterial(materialID);
        if (!material) return;
        
        // Set material properties
        m_currentShader->setVec3("material.diffuse", material->diffuse);
        m_currentShader->setFloat("material.metallic", material->metallic);
        m_currentShader->setFloat("material.roughness", material->roughness);
        m_currentShader->setFloat("material.ao", material->ao);
        
        // Bind textures
        int textureUnit = 0;
        
        if (material->diffuseTexture != INVALID_TEXTURE) {
            GPUTexture* gpuTexture = m_gpuResourceManager->GetGPUTexture(material->diffuseTexture);
            if (gpuTexture) {
                glActiveTexture(GL_TEXTURE0 + textureUnit);
                glBindTexture(GL_TEXTURE_2D, gpuTexture->textureID);
                m_currentShader->setInt("material.diffuse", textureUnit);
                textureUnit++;
            }
        }
        
        // TODO: Bind other textures (specular, normal, etc.)
        // ... similar to diffuse
    }

    bool IsTransparent(const Material& material) {
        // Check if material has transparency
        // This is a simple check - could be more sophisticated
        //return material.diffuse.a < 1.0f;  // Assuming alpha in albedo
        return false;
    }
};