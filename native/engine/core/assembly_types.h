#pragma once

#include <string>
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

}  // namespace engine
