#pragma once

#include "engine/core/gltf_model.h"
#include "engine/core/math_types.h"

namespace engine {

class EngineCore {
public:
    EngineCore();
    ~EngineCore();

    void Update(float deltaTime);
    void Render(const Mat4& viewProj, const Vec3& cameraPos);

    bool LoadCrankshaft(const void* data, size_t size);
    bool LoadPiston(const void* data, size_t size);
    bool LoadConnectingRod(const void* data, size_t size);

private:
    GltfModel crankshaft_;
    GltfModel piston_;
    GltfModel connectingRod_;

    float angle_{0.0f};
    float rpm_{1200.0f};

    // Engine geometry constants
    const float crankRadius_{0.5f};
    const float rodLength_{1.5f};
};

}  // namespace engine
