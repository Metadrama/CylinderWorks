#pragma once

#include <android/asset_manager.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "engine/core/gltf_loader.h"
#include "engine/core/json_utils.h"
#include "engine/core/math_types.h"
#include "engine/core/mesh.h"
#include "engine/core/physics_stub.h"

namespace engine {

struct EnginePart {
    std::string name;
    Mesh mesh;
    Vec3 color{0.7f, 0.7f, 0.7f};
    Mat4 anchorTransform{Mat4::Identity()};
    Mat4 currentTransform{Mat4::Identity()};

    EnginePart() = default;
    EnginePart(const EnginePart&) = delete;
    EnginePart& operator=(const EnginePart&) = delete;
    EnginePart(EnginePart&&) noexcept = default;
    EnginePart& operator=(EnginePart&&) noexcept = default;
};

class EngineAssembly {
public:
    EngineAssembly() = default;
    ~EngineAssembly();

    EngineAssembly(const EngineAssembly&) = delete;
    EngineAssembly& operator=(const EngineAssembly&) = delete;

    bool Load(AAssetManager* assetManager, const std::string& mappingAssetPath);
    void Destroy();

    const std::vector<EnginePart>& Parts() const { return parts_; }
    std::vector<PartAnchor> Anchors() const;
    void ApplyTransforms(const std::vector<PartTransform>& transforms);

private:
    bool LoadPart(AAssetManager* assetManager,
                  const std::string& basePath,
                  const JsonValue& partJson);

    std::vector<EnginePart> parts_;
    std::unordered_map<std::string, size_t> partLookup_;
};

}  // namespace engine
