#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "defines.h"

// Forward declarations
struct Vertex;
struct Texture;

// Asset IDs - type-safe identifiers
using TextureID = u32;
using MaterialID = u32;
using MeshID = u32;
using ModelAssetID = u32;

static constexpr TextureID INVALID_TEXTURE = 0;
static constexpr MaterialID INVALID_MATERIAL = 0;
static constexpr MeshID INVALID_MESH = 0;
static constexpr ModelAssetID INVALID_MODEL = 0;