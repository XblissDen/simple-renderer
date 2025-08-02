#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "main.h"
#include "ecs/component_manager.h"
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
            DrawBatch(batch);
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

        // Set up basic directional light (simple setup)
        m_currentShader->setVec3("dirLight.direction", dirLightDirection);
        m_currentShader->setVec3("dirLight.ambient", dirLightAmbient);
        m_currentShader->setVec3("dirLight.diffuse", dirLightDiffuse);
        m_currentShader->setVec3("dirLight.specular", dirLightSpecular);

        // point lights
        for (int i = 0; i < 4; i++) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            m_currentShader->setVec3(base + ".position", pointLightPositions[i]);
            m_currentShader->setVec3(base + ".ambient", pointLightColors[i].x * 0.1f, pointLightColors[i].y * 0.1f, pointLightColors[i].z * 0.1f);
            m_currentShader->setVec3(base + ".diffuse", pointLightColors[i]);
            m_currentShader->setVec3(base + ".specular", pointLightColors[i]);
            m_currentShader->setFloat(base + ".constant", 1.0f);
            m_currentShader->setFloat(base + ".linear", 0.09f);
            m_currentShader->setFloat(base + ".quadratic", 0.032f);
        }

        // spotLight
        m_currentShader->setVec3("spotLight.position", camera.Position);
        m_currentShader->setVec3("spotLight.direction", camera.Front);
        m_currentShader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        m_currentShader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        m_currentShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        m_currentShader->setFloat("spotLight.constant", 1.0f);
        m_currentShader->setFloat("spotLight.linear", 0.09f);
        m_currentShader->setFloat("spotLight.quadratic", 0.032f);
        m_currentShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        m_currentShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
    }

    void DrawBatch(const RenderBatch& batch) {
        // Bind material
        BindMaterial(batch.materialID);
        
        // Render all commands in this batch
        for (const RenderCommand* command : batch.commands) {
            DrawCommand(*command);
        }
    }

    void BindMaterial(MaterialID materialID) {
        const Material* material = m_assetManager->GetMaterial(materialID);
        if (!material) return;
        
        // Set material properties
        //m_currentShader->setVec3("material.diffuse", material->diffuse);
        //m_currentShader->setFloat("material.metallic", material->metallic);
        //m_currentShader->setFloat("material.roughness", material->roughness);
        //m_currentShader->setFloat("material.ao", material->ao);
        m_currentShader->setFloat("material.shininess", 32.0f);
        
        // Bind textures
        // Bind diffuse texture to texture unit 0
        if (material->diffuseTexture != INVALID_TEXTURE) {
            GPUTexture* gpuTexture = m_gpuResourceManager->GetGPUTexture(material->diffuseTexture);
            if (gpuTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gpuTexture->textureID);
                m_currentShader->setInt("material.texture_diffuse1", 0);
            }
        }
        
        // Bind specular texture to texture unit 1
        if (material->specularTexture != INVALID_TEXTURE) {
            GPUTexture* gpuTexture = m_gpuResourceManager->GetGPUTexture(material->specularTexture);
            if (gpuTexture) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, gpuTexture->textureID);
                m_currentShader->setInt("material.texture_specular1", 1);
            }
        }
    }

    void DrawCommand(const RenderCommand& command) {
        // Get model asset
        const ModelAsset* model = m_assetManager->GetModel(command.modelID);
        if (!model) return;
        
        // Set per-object uniforms
        m_currentShader->setMat4("model", command.worldMatrix);
        m_currentShader->setMat4("normalMatrix", command.normalMatrix);
        
        // Render all meshes in the model
        for (MeshID meshID : model->meshes) {
            GPUMesh* gpuMesh = m_gpuResourceManager->GetGPUMesh(meshID);
            if (!gpuMesh) continue;
            
            // Bind and draw
            glBindVertexArray(gpuMesh->VAO);
            glDrawElements(GL_TRIANGLES, gpuMesh->indexCount, GL_UNSIGNED_INT, 0);
            
            // Update statistics
            m_drawCalls++;
            m_trianglesRendered += gpuMesh->indexCount / 3;
        }
        
        glBindVertexArray(0);
    }

    bool IsTransparent(const Material& material) {
        // Check if material has transparency
        // This is a simple check - could be more sophisticated
        //return material.diffuse.a < 1.0f;  // Assuming alpha in albedo
        return false;
    }
};