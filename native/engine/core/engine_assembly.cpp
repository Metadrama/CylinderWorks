#include "engine/core/engine_assembly.h"

#include <android/asset_manager.h>
#include <android/log.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "engine/core/json_utils.h"
#include "engine/core/math_types.h"

namespace engine {
namespace {
constexpr const char* kTag = "EngineRenderer";

std::string ReadTextAsset(AAssetManager* assetManager, const std::string& path) {
    if (!assetManager) {
        return {};
    }
    AAsset* asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER);

    if (!asset) {
        // Some Flutter releases return keys without the flutter_assets/ prefix.
        if (path.rfind("flutter_assets/", 0) == 0) {
            const std::string trimmed = path.substr(std::strlen("flutter_assets/"));
            asset = AAssetManager_open(assetManager, trimmed.c_str(), AASSET_MODE_BUFFER);
        }
    }

    if (!asset) {
        // Assets inside APKs live under assets/flutter_assets/..., so try re-prefixing.
        if (path.rfind("assets/", 0) != 0) {
            std::string rePrefixed = "assets/" + path;
            asset = AAssetManager_open(assetManager, rePrefixed.c_str(), AASSET_MODE_BUFFER);
        }
    }

    if (!asset) {
        return {};
    }
    const off_t length = AAsset_getLength(asset);
    std::string content;
    content.resize(static_cast<size_t>(length));
    const int64_t read = AAsset_read(asset, content.data(), length);
    AAsset_close(asset);
    if (read != length) {
        return {};
    }
    return content;
}

std::string JoinAssetPath(const std::string& base, const std::string& relative) {
    if (relative.empty()) {
        return relative;
    }

    if (relative[0] == '/' ||
        relative.rfind("assets/", 0) == 0 ||
        relative.rfind("flutter_assets/", 0) == 0) {
        return relative;
    }

    // Default assets live under the bundle "assets/" root.
    return std::string("assets/") + relative;
}

Vec3 ParseVec3(const JsonValue& array, const Vec3& fallback) {
    if (!array.IsArray() || array.Size() != 3) {
        return fallback;
    }
    return Vec3{
        static_cast<float>(array[0].AsNumber(fallback.x)),
        static_cast<float>(array[1].AsNumber(fallback.y)),
        static_cast<float>(array[2].AsNumber(fallback.z))
    };
}

}  // namespace

EngineAssembly::~EngineAssembly() {
    Destroy();
}

bool EngineAssembly::Load(AAssetManager* assetManager, const std::string& mappingAssetPath) {
    Destroy();

    const std::string jsonText = ReadTextAsset(assetManager, mappingAssetPath);
    if (jsonText.empty()) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Failed to read assembly mapping '%s'", mappingAssetPath.c_str());
        return false;
    }

    JsonValue document;
    if (!ParseJson(jsonText, &document)) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Assembly JSON parse error");
        return false;
    }

    const JsonValue& partsValue = document["parts"];
    if (!partsValue.IsArray()) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Assembly mapping missing 'parts' array");
        return false;
    }

    const size_t slash = mappingAssetPath.find_last_of('/');
    const std::string basePath = (slash == std::string::npos) ? std::string{} : mappingAssetPath.substr(0, slash + 1);

    for (size_t i = 0; i < partsValue.Size(); ++i) {
        const JsonValue& partJson = partsValue[i];
        if (!LoadPart(assetManager, basePath, partJson)) {
            __android_log_print(ANDROID_LOG_WARN, kTag, "Unable to load part entry");
        }
    }

    partLookup_.clear();
    for (size_t i = 0; i < parts_.size(); ++i) {
        partLookup_[parts_[i].name] = i;
    }

    return !parts_.empty();
}

void EngineAssembly::Destroy() {
    for (auto& part : parts_) {
        part.mesh.Destroy();
    }
    parts_.clear();
    partLookup_.clear();
}

std::vector<PartAnchor> EngineAssembly::Anchors() const {
    std::vector<PartAnchor> anchors;
    anchors.reserve(parts_.size());
    for (const auto& part : parts_) {
        PartAnchor anchor;
        anchor.name = part.name;
        anchor.transform = part.anchorTransform;
        anchors.push_back(anchor);
    }
    return anchors;
}

void EngineAssembly::ApplyTransforms(const std::vector<PartTransform>& transforms) {
    if (transforms.empty()) {
        for (auto& part : parts_) {
            part.currentTransform = part.anchorTransform;
        }
        return;
    }

    for (auto& part : parts_) {
        part.currentTransform = part.anchorTransform;
    }

    for (const auto& transform : transforms) {
        auto it = partLookup_.find(transform.name);
        if (it == partLookup_.end()) {
            continue;
        }
        parts_[it->second].currentTransform = transform.transform;
    }
}

bool EngineAssembly::LoadPart(AAssetManager* assetManager,
                              const std::string& basePath,
                              const JsonValue& partJson) {
    if (!partJson.Contains("name") || !partJson.Contains("mesh")) {
        return false;
    }

    EnginePart part;
    part.name = partJson["name"].AsString("");
    const std::string meshRelative = partJson["mesh"].AsString("");
    const std::string meshPath = JoinAssetPath(basePath, meshRelative);

    __android_log_print(ANDROID_LOG_INFO, kTag,
                        "Loading part '%s' from asset '%s' (relative '%s')",
                        part.name.c_str(), meshPath.c_str(), meshRelative.c_str());

    Vec3 position{0.0f, 0.0f, 0.0f};
    Vec3 rotation{0.0f, 0.0f, 0.0f};
    Vec3 color{0.75f, 0.75f, 0.75f};

    if (partJson.Contains("anchor")) {
        const JsonValue& anchor = partJson["anchor"];
        if (anchor.Contains("position")) {
            position = ParseVec3(anchor["position"], position);
        }
        if (anchor.Contains("rotationEuler")) {
            rotation = ParseVec3(anchor["rotationEuler"], rotation);
        }
        if (anchor.Contains("color")) {
            color = ParseVec3(anchor["color"], color);
        }
    }

    part.anchorTransform = ComposeTransform(position, rotation);
    part.currentTransform = part.anchorTransform;
    part.color = color;

    MeshSourceData sourceData;
    std::string error;
    if (LoadMeshFromGlb(assetManager, meshPath, &sourceData, &error)) {
        if (!sourceData.positions.empty()) {
            Vec3 minBounds{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
            Vec3 maxBounds{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
            for (size_t i = 0; i + 2 < sourceData.positions.size(); i += 3) {
                const float x = sourceData.positions[i + 0];
                const float y = sourceData.positions[i + 1];
                const float z = sourceData.positions[i + 2];
                minBounds.x = std::min(minBounds.x, x);
                minBounds.y = std::min(minBounds.y, y);
                minBounds.z = std::min(minBounds.z, z);
                maxBounds.x = std::max(maxBounds.x, x);
                maxBounds.y = std::max(maxBounds.y, y);
                maxBounds.z = std::max(maxBounds.z, z);
            }
            __android_log_print(ANDROID_LOG_INFO, kTag,
                                "Part '%s' geometry: %zu vertices, %zu indices, AABB min(%.3f, %.3f, %.3f) max(%.3f, %.3f, %.3f)",
                                part.name.c_str(), sourceData.positions.size() / 3,
                                sourceData.indices.size(),
                                minBounds.x, minBounds.y, minBounds.z,
                                maxBounds.x, maxBounds.y, maxBounds.z);
        } else {
            __android_log_print(ANDROID_LOG_ERROR, kTag,
                                "Part '%s' mesh '%s' returned no vertex positions", part.name.c_str(), meshPath.c_str());
        }

        if (!part.mesh.Initialize(sourceData.positions, sourceData.normals, sourceData.indices)) {
            __android_log_print(ANDROID_LOG_ERROR, kTag,
                                "Part '%s' mesh initialization failed (%s)", part.name.c_str(), meshPath.c_str());
        }
    }

    if (!part.mesh.IsValid()) {
        if (!error.empty()) {
            __android_log_print(ANDROID_LOG_ERROR, kTag,
                                "Failed to load mesh for part '%s' (%s): %s", part.name.c_str(), meshPath.c_str(),
                                error.c_str());
        } else {
            __android_log_print(ANDROID_LOG_ERROR, kTag,
                                "Mesh for part '%s' invalid after load (%s); using placeholder", part.name.c_str(),
                                meshPath.c_str());
        }
        __android_log_print(ANDROID_LOG_WARN, kTag,
                            "Falling back to placeholder mesh for part '%s' (%s)",
                            part.name.c_str(), meshPath.c_str());
        part.mesh.InitializePlaceholder(0.15f);
    }

    parts_.push_back(std::move(part));
    return true;
}

}  // namespace engine
