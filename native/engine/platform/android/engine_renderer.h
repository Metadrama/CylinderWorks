#pragma once

#include <atomic>
#include <array>
#include <mutex>
#include <thread>

#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "engine/core/camera.h"
#include "engine/platform/android/egl_context.h"
#include "engine/core/grid_plane.h"
#include "engine/core/diagnostics.h"
#include "engine/core/math_types.h"
#include "engine/core/shader_program.h"

namespace engine {

class EngineRenderer {
public:
    EngineRenderer();
    ~EngineRenderer();

    bool SetSurface(ANativeWindow* window);
    void ClearSurface();

    void Resize(int width, int height);

    void Orbit(float deltaYaw, float deltaPitch);
    void Pan(float deltaX, float deltaY);
    void Zoom(float scaleDelta);

    void SetPreferredFrameRate(int fps);

    void Start();
    void Stop();

    void FillDiagnostics(DiagnosticsSnapshot* outSnapshot) const;

private:
    void InitializeGlResourcesLocked();
    void DestroyGlResourcesLocked();
    void ClearSurfaceLocked();

    void RenderFrame(int64_t frameTimeNanos);
    static void FrameCallback(long frameTimeNanos, void* data);

    void ScheduleNextFrame();
    void StartFallbackLoopLocked();
    void StopFallbackLoopLocked();
    void FallbackLoop();

    bool isRunning_{false};

    mutable std::mutex mutex_;
    ANativeWindow* window_{nullptr};

    EglContext egl_{};
    OrbitCamera camera_{};
    ShaderProgram shader_{};
    GridPlane gridPlane_{};

    GLint uViewProj_{-1};
    GLint uModel_{-1};
    GLint uCameraPos_{-1};

    int width_{0};
    int height_{0};

    std::atomic<int> preferredFps_{60};

    // Timing
    std::atomic<int64_t> lastFrameTime_{0};
    std::atomic<float> fps_{0.0f};
    std::atomic<float> frameTimeMs_{0.0f};
    std::atomic<int32_t> frameCounter_{0};

    std::array<char, 128> gpuRenderer_{};
    std::array<char, 128> gpuVendor_{};
    std::array<char, 128> gpuVersion_{};

    std::thread fallbackThread_;
    std::atomic_bool fallbackThreadRunning_{false};
};

}  // namespace engine
