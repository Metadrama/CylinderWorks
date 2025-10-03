#pragma once

#include <string>
#include <vector>

#include "engine/core/math_types.h"

namespace engine {

struct EngineControlInputs {
    float throttle{0.0f};
    bool starterEngaged{false};
    bool ignitionEnabled{false};
};

struct PartAnchor {
    std::string name;
    Mat4 transform{Mat4::Identity()};
};

struct PartTransform {
    std::string name;
    Mat4 transform{Mat4::Identity()};
};

class PhysicsSystemStub {
public:
    PhysicsSystemStub() = default;

    void SetAnchors(const std::vector<PartAnchor>& anchors);
    void SetControlInputs(const EngineControlInputs& inputs);

    const EngineControlInputs& Inputs() const { return inputs_; }
    const std::vector<PartTransform>& Evaluate(float deltaSeconds);

private:
    EngineControlInputs inputs_{};
    std::vector<PartTransform> transforms_{};
};

}  // namespace engine
