#include "engine/core/engine_core.h"
#include <cmath>

namespace engine {

EngineCore::EngineCore() = default;
EngineCore::~EngineCore() = default;

void EngineCore::Update(float deltaTime) {
    // rpm = rotations per minute.
    // rotations per second = rpm / 60
    // radians per second = (rpm / 60) * 2 * PI
    const float radiansPerSecond = (rpm_ / 60.0f) * 6.283185307f;
    angle_ += radiansPerSecond * deltaTime;
    if (angle_ > 6.283185307f) {
        angle_ -= 6.283185307f;
    }
}

void EngineCore::Render(const Mat4& viewProj, const Vec3& cameraPos) {
    // Crankshaft: Rotation around Z axis.
    Mat4 crankModel = Mat4::RotateZ(angle_);
    crankshaft_.Draw(viewProj, crankModel, cameraPos);

    // Piston: Translation along Y axis.
    // y = r * cos(theta) + sqrt(L^2 - (r * sin(theta))^2)
    float r = crankRadius_;
    float L = rodLength_;
    float s_theta = std::sin(angle_);
    float c_theta = std::cos(angle_);
    float y = r * c_theta + std::sqrt(L * L - (r * s_theta) * (r * s_theta));
    
    // We might need to subtract the rod length from y to keep it closer to the origin or something.
    // Let's just use it as is for now.
    Mat4 pistonModel = Mat4::Translate(Vec3{0.0f, y, 0.0f});
    piston_.Draw(viewProj, pistonModel, cameraPos);

    // Connecting rod:
    // Positioned halfway? No, it connects (r*sin, r*cos) to (0, y).
    // Midpoint: (r*sin/2, (r*cos + y)/2)
    // Rotation: phi = asin(r/L * sin(theta))
    float phi = std::asin((r / L) * s_theta);
    Mat4 rodModel = Mat4::Translate(Vec3{0.0f, y, 0.0f});
    rodModel = Multiply(rodModel, Mat4::RotateZ(-phi));
    connectingRod_.Draw(viewProj, rodModel, cameraPos);
}

bool EngineCore::LoadCrankshaft(const void* data, size_t size) {
    return crankshaft_.Load(data, size);
}

bool EngineCore::LoadPiston(const void* data, size_t size) {
    return piston_.Load(data, size);
}

bool EngineCore::LoadConnectingRod(const void* data, size_t size) {
    return connectingRod_.Load(data, size);
}

}  // namespace engine
