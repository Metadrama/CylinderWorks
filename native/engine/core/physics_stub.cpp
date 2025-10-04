#include "engine/core/physics_stub.h"

#include <android/log.h>

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <utility>

#include "engine/core/math_types.h"

namespace engine {
namespace {
constexpr const char* kTag = "PhysicsStub";
constexpr size_t kInvalidIndex = static_cast<size_t>(-1);
}

void PhysicsSystemStub::SetAnchors(const std::vector<PartAnchor>& anchors) {
    struct Node {
        std::string name;
        std::string parentName;
        size_t parentIndex{kInvalidIndex};
        bool hasRelative{false};
        Mat4 relative{Mat4::Identity()};
        Mat4 fallback{Mat4::Identity()};
    };

    transforms_.clear();
    if (anchors.empty()) {
        return;
    }

    std::vector<Node> nodes;
    nodes.reserve(anchors.size());
    std::unordered_map<std::string, size_t> lookup;
    lookup.reserve(anchors.size());

    for (const auto& anchor : anchors) {
        Node node;
        node.name = anchor.name;
        node.parentName = anchor.parentName;
        node.fallback = anchor.defaultTransform;
        if (!anchor.attachmentName.empty()) {
            node.relative = CombineAttachmentTransforms(anchor.parentAttachment, anchor.selfAttachment);
            node.hasRelative = true;
        } else if (!anchor.parentName.empty()) {
            __android_log_print(ANDROID_LOG_WARN, kTag,
                                "Anchor '%s' specifies parent '%s' without attachment name",
                                anchor.name.c_str(), anchor.parentName.c_str());
        }
        lookup[node.name] = nodes.size();
        nodes.push_back(std::move(node));
    }

    for (size_t i = 0; i < nodes.size(); ++i) {
        Node& node = nodes[i];
        if (node.parentName.empty()) {
            continue;
        }
        auto it = lookup.find(node.parentName);
        if (it == lookup.end()) {
            __android_log_print(ANDROID_LOG_ERROR, kTag,
                                "Anchor '%s' references missing parent '%s'",
                                node.name.c_str(), node.parentName.c_str());
            continue;
        }
        if (it->second == i) {
            __android_log_print(ANDROID_LOG_ERROR, kTag,
                                "Anchor '%s' cannot be its own parent", node.name.c_str());
            continue;
        }
        node.parentIndex = it->second;
        if (!node.hasRelative) {
            __android_log_print(ANDROID_LOG_WARN, kTag,
                                "Anchor '%s' missing attachment transforms for parent '%s'",
                                node.name.c_str(), node.parentName.c_str());
        }
    }

    std::vector<Mat4> resolved(nodes.size(), Mat4::Identity());
    std::vector<uint8_t> state(nodes.size(), 0);

    std::function<Mat4(size_t)> resolve = [&](size_t index) -> Mat4 {
        if (index >= nodes.size()) {
            return Mat4::Identity();
        }
        if (state[index] == 2) {
            return resolved[index];
        }
        if (state[index] == 1) {
            __android_log_print(ANDROID_LOG_ERROR, kTag,
                                "Cycle detected when resolving anchor '%s'",
                                nodes[index].name.c_str());
            return resolved[index];
        }

        state[index] = 1;
        Mat4 transform = nodes[index].fallback;
        if (nodes[index].parentIndex != kInvalidIndex) {
            const Mat4 parentTransform = resolve(nodes[index].parentIndex);
            if (nodes[index].hasRelative) {
                transform = Multiply(parentTransform, nodes[index].relative);
            }
        }
        resolved[index] = transform;
        state[index] = 2;
        return transform;
    };

    transforms_.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        PartTransform transform;
        transform.name = nodes[i].name;
        transform.transform = resolve(i);
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
