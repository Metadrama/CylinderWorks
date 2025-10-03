#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <android/asset_manager.h>

namespace engine {

struct MeshSourceData {
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<uint32_t> indices;
};

bool LoadMeshFromGlb(AAssetManager* assetManager,
                     const std::string& assetPath,
                     MeshSourceData* outData,
                     std::string* outError);

}  // namespace engine
