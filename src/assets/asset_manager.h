#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "defines.h"

// Forward declarations
//struct Vertex;
//struct Texture;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

// Asset IDs - type-safe identifiers
using TextureID = u32;
using MaterialID = u32;
using MeshID = u32;
using ModelAssetID = u32;

static constexpr TextureID INVALID_TEXTURE = 0;
static constexpr MaterialID INVALID_MATERIAL = 0;
static constexpr MeshID INVALID_MESH = 0;
static constexpr ModelAssetID INVALID_MODEL = 0;

struct MeshData{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::string name;

    // bounding info for culling
    glm::vec3 boundsMin{0.0f};
    glm::vec3 boundsMax{0.0f};
    glm::vec3 boundsCenter{0.0f};
    f32 boundsRadius = 0.0f;
};

struct TextureData{
    std::string path;
    std::string type;

    // Image data (before GPU upload)
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;

    // GPU resource ID (set by renderer)
    u32 glTextureID = 0;
    bool isLoaded = false;
};

struct Material{
    std::string name;

    // PBR properties
    glm::vec3 diffuse{1.0f};
    f32 metallic = 0.0f;
    f32 roughness = 0.5f;
    f32 ao = 1.0f;

    // Texture references
    TextureID albedoTexture = INVALID_TEXTURE;
    TextureID normalTexture = INVALID_TEXTURE;
    TextureID metallicTexture = INVALID_TEXTURE;
    TextureID roughnessTexture = INVALID_TEXTURE;
    TextureID aoTexture = INVALID_TEXTURE;
    
    // Legacy support
    TextureID diffuseTexture = INVALID_TEXTURE;
    TextureID specularTexture = INVALID_TEXTURE;
};

struct ModelAsset{
    std::string path;
    std::string name;
    std::vector<MeshID> meshes;
    std::vector<MaterialID> materials;

    // Model-wide bounding info
    glm::vec3 boundsMin{0.0f};
    glm::vec3 boundsMax{0.0f};
    glm::vec3 boundsCenter{0.0f};
    f32 boundsRadius = 0.0f;
};

// Asset loading stats
struct AssetStats{
    u32 texturesLoaded = 0;
    u32 materialsCreated = 0;
    u32 meshesLoaded = 0;
    u32 modelsLoaded = 0;
    u32 totalVertices = 0;
    u32 totalTriangles = 0;
    size_t memoryUsed = 0; // Bytes
};

class AssetManager{
private:
    // Asset storage
    std::vector<TextureData> m_textures;
    std::vector<Material> m_materials;
    std::vector<MeshData> m_meshes;
    std::vector<ModelAsset> m_models;

    // Path-to-ID mapping for duplicate prevention
    std::unordered_map<std::string, TextureID> m_texturePathMap;
    std::unordered_map<std::string, ModelAssetID> m_modelPathMap;
    std::unordered_map<std::string, MaterialID> m_materialNameMap;

    // Next available IDs
    TextureID m_nextTextureID = 1;
    MaterialID m_nextMaterialID = 1;
    MeshID m_nextMeshID = 1;
    ModelAssetID m_nextModelID = 1;

    // Loading statistics
    AssetStats m_stats;

    // Assimp importer (reused for efficiency)
    Assimp::Importer m_importer;

public:
    AssetManager(){
        // Reserve space for common asset counts
        m_textures.reserve(1024);
        m_materials.reserve(256);
        m_meshes.reserve(2048);
        m_models.reserve(128);
    }

    ~AssetManager(){
        // Free texture data
        for(auto& texture: m_textures){
            if(texture.data) stbi_image_free(texture.data);
        }
    }

    // Model Loading
    ModelAssetID LoadModel(const std::string& path){
        // Check if already loaded
        auto it = m_modelPathMap.find(path);
        if(it != m_modelPathMap.end()){
            return it->second;
        }

        // Load with Assimp
        const aiScene* scene = m_importer.ReadFile(path,
            aiProcess_Triangulate | aiProcess_FlipUVs | 
            aiProcess_CalcTangentSpace| aiProcess_GenNormals 
        );

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            // Log error
            return INVALID_MODEL;
        }

        // Create model asset
        ModelAssetID modelID = m_nextModelID++;
        if(modelID >= m_models.size()){
            m_models.resize(modelID + 1);
        }

        ModelAsset& model = m_models[modelID];
        model.path = path;
        model.name = ExtractFileName(path);

        // Extract directory for texture loading
        std::string directory = path.substr(0, path.find_last_of('/'));

        // Process all materials first
        ProcessMaterials(scene, directory, model);

        // Process all meshes
        ProcessNode(scene->mRootNode, scene, model);

        // Calculate model bound
        CalculateModelBounds(model);

        // Cache the loaded model
        m_modelPathMap[path] = modelID;
        m_stats.modelsLoaded++;

        return modelID;
    }

    // Texture loading
    TextureID LoadTexture(const std::string& path, const std::string& type = "diffuse"){
        auto it = m_texturePathMap.find(path);
        if(it != m_texturePathMap.end()) return it->second;

        TextureID textureID = m_nextTextureID++;
        if(textureID >= m_textures.size()){
            m_textures.resize(textureID + 1);
        }

        TextureData& texture = m_textures[textureID];
        texture.path = path;
        texture.type = type;

        // Load Image data (CPU side)
        texture.data = stbi_load(path.c_str(), &texture.width, &texture.height, &texture.channels, 0);
        if(!texture.data){
            // Log error, return invalid
            return INVALID_TEXTURE;
        }

        // Cache and update stats
        m_texturePathMap[path] = textureID;
        m_stats.texturesLoaded++;
        m_stats.memoryUsed += texture.width * texture.height * texture.channels;

        return textureID;
    }

    // Material creation
    MaterialID CreateMaterial(const std::string& name = ""){
        MaterialID materialID = m_nextMaterialID++;
        if(materialID >= m_materials.size()){
            m_materials.resize(materialID + 1);
        }

        Material& material = m_materials[materialID];
        material.name = name.empty() ? ("Material_" + std::to_string(materialID)) : name;

        if(!name.empty()){
            m_materialNameMap[name] = materialID;
        }

        m_stats.materialsCreated++;
        return materialID;
    }

    // Asset access
    const ModelAsset* GetModel(ModelAssetID id) const {
        return (id > 0 && id < m_models.size()) ? &m_models[id] : nullptr;
    }
    
    const MeshData* GetMesh(MeshID id) const {
        return (id > 0 && id < m_meshes.size()) ? &m_meshes[id] : nullptr;
    }
    
    const Material* GetMaterial(MaterialID id) const {
        return (id > 0 && id < m_materials.size()) ? &m_materials[id] : nullptr;
    }
    
    const TextureData* GetTexture(TextureID id) const {
        return (id > 0 && id < m_textures.size()) ? &m_textures[id] : nullptr;
    }
    
    // Mutable access for renderer
    TextureData* GetTexture(TextureID id) {
        return (id > 0 && id < m_textures.size()) ? &m_textures[id] : nullptr;
    }

    // Statistics
    const AssetStats& GetStats() const { return m_stats; }

    // Resource management
    void UnloadModel(ModelAssetID id) {
        // Implementation: Mark as unused, cleanup if no references
        // This is complex - need reference counting
    }

private:
    void ProcessMaterials(const aiScene* scene, const std::string& directory, ModelAsset& model){
        printf("num materials %d", scene->mNumMaterials);
        for (size_t i = 0; i < scene->mNumMaterials; i++)
        {
            aiMaterial* aiMat = scene->mMaterials[i];

            MaterialID materialID = CreateMaterial();
            Material& material = m_materials[materialID];

            // Load material properties
            aiColor3D color;
            if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
                material.diffuse = glm::vec3(color.r, color.g, color.b);
            }
            
            float shininess;
            if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
                material.roughness = 1.0f - (shininess / 128.0f);  // Convert to roughness
            }
            
            // Load textures
            material.diffuseTexture = LoadMaterialTextures(aiMat, aiTextureType_DIFFUSE, directory);
            material.specularTexture = LoadMaterialTextures(aiMat, aiTextureType_SPECULAR, directory);
            material.normalTexture = LoadMaterialTextures(aiMat, aiTextureType_NORMALS, directory);
            
            model.materials.push_back(materialID);
        }
        
    }

    void ProcessNode(aiNode* node, const aiScene* scene, ModelAsset& model){
        // Process all meshes in this node
        for(size_t i = 0; i < node->mNumMeshes; i++){
            aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];
            MeshID meshID = ProcessMesh(aiMesh, scene, model);
            if(meshID != INVALID_MESH){
                model.meshes.push_back(meshID);
            }
        }

        // Process child nodes
        for(size_t i = 0; i < node->mNumChildren; i++){
            ProcessNode(node->mChildren[i], scene, model);
        }
    }

    MeshID ProcessMesh(aiMesh* aiMesh, const aiScene* scene, ModelAsset& model){
        MeshID meshID = m_nextMeshID++;
        if (meshID >= m_meshes.size()) {
            m_meshes.resize(meshID + 1);
        }

        MeshData& mesh = m_meshes[meshID];
        mesh.name = aiMesh->mName.C_Str();

        // Process vertices
        mesh.vertices.reserve(aiMesh->mNumVertices);

        // Process vertices
        mesh.vertices.reserve(aiMesh->mNumVertices);
        for (uint32_t i = 0; i < aiMesh->mNumVertices; i++) {
            Vertex vertex;
            
            // Position
            vertex.Position.x = aiMesh->mVertices[i].x;
            vertex.Position.y = aiMesh->mVertices[i].y;
            vertex.Position.z = aiMesh->mVertices[i].z;
            
            // Normal
            if (aiMesh->HasNormals()) {
                vertex.Normal.x = aiMesh->mNormals[i].x;
                vertex.Normal.y = aiMesh->mNormals[i].y;
                vertex.Normal.z = aiMesh->mNormals[i].z;
            }
            
            // Texture coordinates
            if (aiMesh->mTextureCoords[0]) {
                vertex.TexCoords.x = aiMesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = aiMesh->mTextureCoords[0][i].y;
            }
            
            mesh.vertices.push_back(vertex);
        }

        // Process indices
        mesh.indices.reserve(aiMesh->mNumFaces * 3);
        for (uint32_t i = 0; i < aiMesh->mNumFaces; i++) {
            aiFace face = aiMesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++) {
                mesh.indices.push_back(face.mIndices[j]);
            }
        }

        // Calculate mesh bounds
        CalculateMeshBounds(mesh);
        
        // Update statistics
        m_stats.meshesLoaded++;
        m_stats.totalVertices += mesh.vertices.size();
        m_stats.totalTriangles += mesh.indices.size() / 3;
        m_stats.memoryUsed += mesh.vertices.size() * sizeof(Vertex);
        m_stats.memoryUsed += mesh.indices.size() * sizeof(uint32_t);
        
        return meshID;
    }

    TextureID LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& directory) {
        if (mat->GetTextureCount(type) == 0) {
            return INVALID_TEXTURE;
        }
        
        aiString str;
        mat->GetTexture(type, 0, &str);  // Get first texture of this type
        
        std::string fullPath = directory + "/" + str.C_Str();
        return LoadTexture(fullPath, GetTextureTypeName(type));
    }

    std::string GetTextureTypeName(aiTextureType type) {
        switch (type) {
            case aiTextureType_DIFFUSE: return "diffuse";
            case aiTextureType_SPECULAR: return "specular";
            case aiTextureType_NORMALS: return "normal";
            case aiTextureType_HEIGHT: return "height";
            default: return "unknown";
        }
    }

    void CalculateMeshBounds(MeshData& mesh) {
        if (mesh.vertices.empty()) return;
        
        mesh.boundsMin = mesh.boundsMax = mesh.vertices[0].Position;
        
        for (const auto& vertex : mesh.vertices) {
            mesh.boundsMin = glm::min(mesh.boundsMin, vertex.Position);
            mesh.boundsMax = glm::max(mesh.boundsMax, vertex.Position);
        }
        
        mesh.boundsCenter = (mesh.boundsMin + mesh.boundsMax) * 0.5f;
        mesh.boundsRadius = glm::distance(mesh.boundsCenter, mesh.boundsMax);
    }

    void CalculateModelBounds(ModelAsset& model) {
        if (model.meshes.empty()) return;
        
        bool first = true;
        for (MeshID meshID : model.meshes) {
            const MeshData& mesh = m_meshes[meshID];
            
            if (first) {
                model.boundsMin = mesh.boundsMin;
                model.boundsMax = mesh.boundsMax;
                first = false;
            } else {
                model.boundsMin = glm::min(model.boundsMin, mesh.boundsMin);
                model.boundsMax = glm::max(model.boundsMax, mesh.boundsMax);
            }
        }
        
        model.boundsCenter = (model.boundsMin + model.boundsMax) * 0.5f;
        model.boundsRadius = glm::distance(model.boundsCenter, model.boundsMax);
    }

    std::string ExtractFileName(const std::string& path){
        size_t pos = path.find_last_of("/\\");
        if(pos == std::string::npos) return path;

        std::string filename = path.substr(pos + 1);
        size_t dotPos = filename.find_last_of('.');
        if(dotPos != std::string::npos){
            filename = filename.substr(0, dotPos);
        }

        return filename;
    }
};