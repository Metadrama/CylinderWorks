#include "engine/core/physics_stub.h"

namespace engine {

void PhysicsSystemStub::SetAnchors(const std::vector<PartAnchor>& anchors) {
    transforms_.clear();
    transforms_.reserve(anchors.size());
    for (const auto& anchor : anchors) {
        PartTransform transform;
        transform.name = anchor.name;
        transform.transform = anchor.transform;
        transforms_.push_back(transform);
    }
}

void PhysicsSystemStub::SetControlInputs(const EngineControlInputs& inputs) {
    inputs_ = inputs;
}

const std::vector<PartTransform>& PhysicsSystemStub::Evaluate(float /*deltaSeconds*/) {
    // Stub: mirror anchors so the renderer has stable transforms until the
    // real dynamics system is integrated.
    return transforms_;
}

}  // namespace engine
