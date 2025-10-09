#include "engine/core/kinematics_system.h"

#include <android/log.h>

#include <utility>

namespace engine {
namespace {
constexpr const char* kTag = "EngineRenderer";
}

bool KinematicsSystem::Initialize(const std::vector<PartAnchor>& anchors,
                                  const std::vector<AssemblyConstraint>& constraints) {
    anchors_ = anchors;
    constraints_ = constraints;
    anchorLookup_.clear();

    for (size_t i = 0; i < anchors_.size(); ++i) {
        const std::string& name = anchors_[i].name;
        if (name.empty()) {
            __android_log_print(ANDROID_LOG_WARN, kTag,
                                "Kinematics anchor at index %zu is missing a name", i);
            continue;
        }
        anchorLookup_[name] = i;
    }

    return !anchors_.empty() || !constraints_.empty();
}

std::vector<PartTransform> KinematicsSystem::BuildDefaultPose() const {
    std::vector<PartTransform> transforms;
    transforms.reserve(anchors_.size());
    for (const auto& anchor : anchors_) {
        PartTransform transform;
        transform.name = anchor.name;
        transform.transform = anchor.defaultTransform;
        transforms.push_back(transform);
    }
    return transforms;
}

}  // namespace engine
