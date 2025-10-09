#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "engine/core/assembly_types.h"

namespace engine {

class KinematicsSystem {
public:
    KinematicsSystem() = default;

    bool Initialize(const std::vector<PartAnchor>& anchors,
                    const std::vector<AssemblyConstraint>& constraints);

    std::vector<PartTransform> BuildDefaultPose() const;

    const std::vector<PartAnchor>& Anchors() const { return anchors_; }
    const std::vector<AssemblyConstraint>& Constraints() const { return constraints_; }
    size_t ConstraintCount() const { return constraints_.size(); }

private:
    std::vector<PartAnchor> anchors_;
    std::vector<AssemblyConstraint> constraints_;
    std::unordered_map<std::string, size_t> anchorLookup_;
};

}  // namespace engine
