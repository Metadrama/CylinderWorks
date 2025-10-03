#pragma once

#include "math_types.h"

namespace engine {

class OrbitCamera {
public:
    OrbitCamera();

    void Reset();
    void Orbit(float deltaYaw, float deltaPitch);
    void Pan(float deltaX, float deltaY);
    void Zoom(float deltaDistance);

    void SetViewport(int width, int height);

    Mat4 ViewMatrix() const;
    Mat4 ProjectionMatrix() const;
    Vec3 EyePosition() const;
    Vec3 Target() const { return target_; }

    float Distance() const { return distance_; }
    float Yaw() const { return yaw_; }
    float Pitch() const { return pitch_; }

private:
    void ClampAngles();
    void ClampDistance();

    Vec3 target_{0.0f, 0.0f, 0.0f};
    float distance_{6.0f};
    float yaw_{0.78539816339f};  // 45 degrees
    float pitch_{0.610865238f};  // ~35 degrees
    int viewportWidth_{1};
    int viewportHeight_{1};

    float minDistance_{0.5f};
    float maxDistance_{50.0f};
};

}  // namespace engine
