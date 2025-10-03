#include "camera.h"

#include <algorithm>

namespace engine {

namespace {
constexpr float kPitchMin = -1.3962634f;  // -80 degrees
constexpr float kPitchMax = 1.3962634f;   // 80 degrees
constexpr float kOrbitSensitivity = 0.005f;
constexpr float kPanSensitivity = 0.0025f;
constexpr float kZoomSensitivity = 1.0f;
}  // namespace

OrbitCamera::OrbitCamera() {
    Reset();
}

void OrbitCamera::Reset() {
    target_ = Vec3{0.0f, 0.0f, 0.0f};
    distance_ = 6.0f;
    yaw_ = 0.78539816339f;   // 45 degrees
    pitch_ = 0.6108652382f;  // 35 degrees
    viewportWidth_ = 1;
    viewportHeight_ = 1;
}

void OrbitCamera::Orbit(float deltaYaw, float deltaPitch) {
    yaw_ += deltaYaw * kOrbitSensitivity;
    pitch_ += deltaPitch * kOrbitSensitivity;
    ClampAngles();
}

void OrbitCamera::Pan(float deltaX, float deltaY) {
    // Translate along camera's right and up axes.
    const Vec3 eye = Vec3{
        target_.x + distance_ * std::cos(pitch_) * std::sin(yaw_),
        target_.y + distance_ * std::sin(pitch_),
        target_.z + distance_ * std::cos(pitch_) * std::cos(yaw_)
    };

    const Vec3 forward = Normalize(target_ - eye);
    const Vec3 worldUp{0.0f, 1.0f, 0.0f};
    const Vec3 right = Normalize(Cross(forward, worldUp));
    const Vec3 up = Normalize(Cross(right, forward));

    const float scale = distance_ * kPanSensitivity;
    target_ += right * (-deltaX * scale);
    target_ += up * (deltaY * scale);
}

void OrbitCamera::Zoom(float deltaDistance) {
    distance_ *= std::exp(-deltaDistance * kZoomSensitivity);
    ClampDistance();
}

void OrbitCamera::SetViewport(int width, int height) {
    viewportWidth_ = std::max(1, width);
    viewportHeight_ = std::max(1, height);
}

Mat4 OrbitCamera::ViewMatrix() const {
    const Vec3 eye = EyePosition();
    const Vec3 up{0.0f, 1.0f, 0.0f};
    return LookAt(eye, target_, up);
}

Mat4 OrbitCamera::ProjectionMatrix() const {
    const float aspect = static_cast<float>(viewportWidth_) / static_cast<float>(viewportHeight_);
    return Perspective(Radians(50.0f), aspect, 0.1f, 500.0f);
}

Vec3 OrbitCamera::EyePosition() const {
    return Vec3{
        target_.x + distance_ * std::cos(pitch_) * std::sin(yaw_),
        target_.y + distance_ * std::sin(pitch_),
        target_.z + distance_ * std::cos(pitch_) * std::cos(yaw_)
    };
}

void OrbitCamera::ClampAngles() {
    if (yaw_ > 3.14159265f) {
        yaw_ -= 6.2831853f;
    } else if (yaw_ < -3.14159265f) {
        yaw_ += 6.2831853f;
    }
    pitch_ = Clamp(pitch_, kPitchMin, kPitchMax);
}

void OrbitCamera::ClampDistance() {
    distance_ = Clamp(distance_, minDistance_, maxDistance_);
}

}  // namespace engine
