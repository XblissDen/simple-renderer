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
#include "shader.h"
#include "defines.h"

// GPU resource handles
struct GPUMesh{
    u32 VAO = 0;
    u32 VBO = 0;
    u32 EBO = 0;
    u32 indexCount = 0;
    bool isUploaded = false;

    ~GPUMesh(){
        if(isUploaded){
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }
    }
};

struct GPUTexture{
    u32 textureID = 0;
    GLenum target = GL_TEXTURE_2D;
    bool isUploaded = false;

    ~GPUTexture(){
        if(isUploaded){
            glDeleteTextures(1, &textureID);
        }
    }
};

// Render Command - what to draw this frame
struct RenderCommand{
    // Transform
    glm::mat4 worldMatrix{1.0f};
    glm::mat4 normalMatrix{1.0f}; // For normal transformation

    // Asset reference
    ModelAssetID modelID = INVALID_MODEL;
    MaterialID materialID = INVALID_MATERIAL;

    // Renrering properties
    float distanceToCamera = 0.0f; // For sorting
    bool castShadows = true;
    bool receiveShadows = true;

    // Optional entity reference (for debugging)
    EntityID entityID = INVALID_ENTITY;
};

// Render batch - multiple objects with same material
struct RenderBatch{
    MaterialID materialID;
    std::vector<RenderCommand*> commands;

    // Sorting for transparency
    bool isTransparent = false;
};

class GPUResourceManager{
private:
    AssetManager* m_assetManager;

    // GPU resource caches
    std::unordered_map<MeshID, std::unique_ptr<GPUMesh>> m_gpuMeshes;
    std::unordered_map<TextureID, std::unique_ptr<GPUTexture>> m_gpuTextures;

    // Statistics
    u32 m_meshesUploaded = 0;
    u32 m_texturesUploaded = 0;
    size_t m_gpuMemoryUsed = 0;

public:
    GPUResourceManager(AssetManager* assetManager): m_assetManager(assetManager){}

    // Get or create GPU mesh
    GPUMesh* GetGPUMesh(MeshID meshID){
        auto it = m_gpuMeshes.find(meshID);
        if(it != m_gpuMeshes.end()){
            return it->second.get();
        }

        // Upload mesh to GPU
        const MeshData* meshData = m_assetManager->GetMesh(meshID);
        if(!meshData){
            return nullptr;
        }

        auto gpuMesh = std::make_unique<GPUMesh>();
        if(UploadMesh(*meshData, *gpuMesh)){
            GPUMesh* result = gpuMesh.get();
            m_gpuMeshes[meshID] = std::move(gpuMesh);
            return result;
        }

        return nullptr;
    }

    // Get or create GPU texture
    GPUTexture* GetGPUTexture(TextureID textureID){
        if(textureID == INVALID_TEXTURE){
            return nullptr;
        }

        auto it = m_gpuTextures.find(textureID);
        if(it != m_gpuTextures.end()){
            return it->second.get();
        }

        // Upload texture to GPU
        TextureData* textureData = m_assetManager->GetTexture(textureID);
        if(!textureData || !textureData->data){
            return nullptr;
        }

        auto gpuTexture = std::make_unique<GPUTexture>();
        if(UploadTexture(*textureData, *gpuTexture)){
            GPUTexture* result = gpuTexture.get();
            m_gpuTextures[textureID] = std::move(gpuTexture);
            return result;
        }

        return nullptr;
    }

    // Force upload all assets for a model (useful for preloading)
    void PreloadModel(ModelAssetID modelID){
        const ModelAsset* model = m_assetManager->GetModel(modelID);
        if(!model) return;

        // Upload all meshes
        for(MeshID meshID: model->meshes){
            GetGPUMesh(meshID);
        }

        // Upload all material textures
        for(MaterialID materialID: model->materials){
            const Material* material = m_assetManager->GetMaterial(materialID);
            if(material){
                GetGPUTexture(material->diffuseTexture);
                GetGPUTexture(material->specularTexture);
                GetGPUTexture(material->normalTexture);
                // TODO: ... other texture types
            }
        }
    }

    // Statistics
    uint32_t GetMeshesUploaded() const { return m_meshesUploaded; }
    uint32_t GetTexturesUploaded() const { return m_texturesUploaded; }
    size_t GetGPUMemoryUsed() const { return m_gpuMemoryUsed; }

private:
    bool UploadMesh(const MeshData& meshData, GPUMesh& gpuMesh){
        // Generate OpenGL objects
        glGenVertexArrays(1, &gpuMesh.VAO);
        glGenBuffers(1, &gpuMesh.VBO);
        glGenBuffers(1, &gpuMesh.EBO);

        // Bind VAO
        glBindVertexArray(gpuMesh.VAO);

        // Upload Vertex data
        glBindBuffer(GL_ARRAY_BUFFER, gpuMesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, meshData.vertices.size() * sizeof(Vertex),
        meshData.vertices.data(), GL_STATIC_DRAW);

        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuMesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                     meshData.indices.size() * sizeof(u32), 
                     meshData.indices.data(), 
                     GL_STATIC_DRAW);
        
        // Set up vertex attributes
        // Position (location 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                              (void*)offsetof(Vertex, Position));
        
        // Normal (location 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                              (void*)offsetof(Vertex, Normal));
        
        // Texture Coordinates (location 2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                              (void*)offsetof(Vertex, TexCoords));

        // Unbind
        glBindVertexArray(0);

        // Store info
        gpuMesh.indexCount = meshData.indices.size();
        gpuMesh.isUploaded = true;

        // Update statistics
        m_meshesUploaded++;
        m_gpuMemoryUsed += meshData.vertices.size() * sizeof(Vertex);
        m_gpuMemoryUsed += meshData.indices.size() * sizeof(uint32_t);
        
        return true;
    }

    bool UploadTexture(TextureData& textureData, GPUTexture& gpuTexture){
        glGenTextures(1, &gpuTexture.textureID);
        glBindTexture(GL_TEXTURE_2D, gpuTexture.textureID);

        // Determine format
        GLenum format;
        if (textureData.channels == 1)
            format = GL_RED;
        else if (textureData.channels == 3)
            format = GL_RGB;
        else if (textureData.channels == 4)
            format = GL_RGBA;
        else {
            glDeleteTextures(1, &gpuTexture.textureID);
            return false;
        }

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, 
                     textureData.width, textureData.height, 0, 
                     format, GL_UNSIGNED_BYTE, textureData.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        // Free CPU data after upload
        if (textureData.data) {
            stbi_image_free(textureData.data);
            textureData.data = nullptr;
        }

        gpuTexture.isUploaded = true;

        // Update statistics  
        m_texturesUploaded++;
        m_gpuMemoryUsed += textureData.width * textureData.height * textureData.channels;
        
        return true;
    }
};
