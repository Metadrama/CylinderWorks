#pragma once

#include <android/asset_manager.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "engine/core/assembly_types.h"
#include "engine/core/gltf_loader.h"
#include "engine/core/json_utils.h"
#include "engine/core/math_types.h"
#include "engine/core/mesh.h"

namespace engine {

struct EnginePart {
    std::string name;
    Mesh mesh;
    Vec3 color{0.7f, 0.7f, 0.7f};
    Mat4 anchorTransform{Mat4::Identity()};
    Mat4 currentTransform{Mat4::Identity()};
    std::string parentName;
    std::string parentAttachmentName;
    size_t parentIndex{static_cast<size_t>(-1)};
    Mat4 relativeAttachment{Mat4::Identity()};
    bool hasRelativeAttachment{false};
    struct AttachmentPair {
        Mat4 self{Mat4::Identity()};
        Mat4 parent{Mat4::Identity()};
    };
    std::unordered_map<std::string, AttachmentPair> attachments;

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
    const std::vector<AssemblyConstraint>& Constraints() const { return constraints_; }
    void ApplyTransforms(const std::vector<PartTransform>& transforms);

private:
    bool LoadPart(AAssetManager* assetManager,
                  const std::string& basePath,
                  const JsonValue& partJson);

    Mat4 ResolveDefaultTransform(size_t index,
                                 std::vector<Mat4>* cache,
                                 std::vector<uint8_t>* state) const;
    std::vector<Mat4> BuildDefaultTransforms() const;

    std::vector<EnginePart> parts_;
    std::unordered_map<std::string, size_t> partLookup_;
    std::vector<AssemblyConstraint> constraints_;
};

}  // namespace engine
