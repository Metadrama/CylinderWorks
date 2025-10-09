#pragma once

#include <string>
#include <vector>

#include "engine/core/math_types.h"

namespace engine {

struct PartAnchor {
    std::string name;
    std::string parentName;
    std::string attachmentName;
    Mat4 defaultTransform{Mat4::Identity()};
    Mat4 selfAttachment{Mat4::Identity()};
    Mat4 parentAttachment{Mat4::Identity()};
};

struct PartTransform {
    std::string name;
    Mat4 transform{Mat4::Identity()};
};

struct ConstraintGeometry {
    std::string geometryType;
    std::vector<std::string> instancePath;
    std::string instanceUid;
    std::string partName;
    std::string entityUid;
    Vec3 position{0.0f, 0.0f, 0.0f};
    Vec3 axis{0.0f, 0.0f, 1.0f};
    bool ground{false};
};

struct AssemblyConstraint {
    std::string name;
    std::string type;
    std::vector<ConstraintGeometry> geometries;
};

}  // namespace engine
