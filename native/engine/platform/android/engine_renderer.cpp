#include "engine/platform/android/engine_renderer.h"

#include <android/choreographer.h>
#include <android/log.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <thread>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace engine {

namespace {
constexpr const char* kTag = "EngineRenderer";
constexpr float kMajorStep = 1.0f;
constexpr float kMinorStep = 0.1f;
constexpr float kPlaneExtent = 200.0f;
constexpr float kLightDir[3] = {-0.35f, 1.0f, 0.45f};

void CopyGlString(const GLubyte* source, std::array<char, 128>& destination) {
    if (!source) {
        destination[0] = '\0';
        return;
    }
    std::snprintf(destination.data(), destination.size(), "%s", reinterpret_cast<const char*>(source));
}

const char* kGridVertexShaderSrc = R"(
#version 300 es
layout(location = 0) in vec3 aPosition;
uniform mat4 uViewProj;
uniform mat4 uModel;
uniform float uExtent;
out vec3 vWorldPos;
out vec3 vLocalPos;
void main() {
    vec4 world = uModel * vec4(aPosition.x * uExtent, aPosition.y, aPosition.z * uExtent, 1.0);
    vWorldPos = world.xyz;
    vLocalPos = aPosition;
    gl_Position = uViewProj * world;
}
)";

const char* kGridFragmentShaderSrc = R"(
#version 300 es
precision mediump float;
in vec3 vWorldPos;
in vec3 vLocalPos;
uniform vec3 uCameraPos;
uniform float uMajorStep;
uniform float uMinorStep;
out vec4 fragColor;

float gridLine(float coord, float stepSize) {
    float coordScaled = coord / stepSize;
    float derivative = fwidth(coordScaled);
    float line = abs(fract(coordScaled - 0.5) - 0.5) / max(derivative, 1e-4);
    return 1.0 - clamp(line, 0.0, 1.0);
}

void main() {
    float minor = max(gridLine(vWorldPos.x, uMinorStep), gridLine(vWorldPos.z, uMinorStep));
    float major = max(gridLine(vWorldPos.x, uMajorStep), gridLine(vWorldPos.z, uMajorStep));

    vec3 baseColor = vec3(0.04, 0.05, 0.07);
    vec3 minorColor = vec3(0.10, 0.12, 0.18);
    vec3 majorColor = vec3(0.35, 0.40, 0.55);

    float blend = max(minor * 0.6, major);
    vec3 color = mix(baseColor, minorColor, minor * 0.7);
    color = mix(color, majorColor, major);

    float fade = clamp(1.0 - length(vLocalPos.xz) * 0.5, 0.0, 1.0);
    color *= fade + 0.2;

    fragColor = vec4(color, 1.0);
}
)";

const char* kPartVertexShaderSrc = R"(
#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
uniform mat4 uPartViewProj;
uniform mat4 uPartModel;
out vec3 vNormal;
out vec3 vWorldPos;
void main() {
    vec4 world = uPartModel * vec4(aPosition, 1.0);
    vWorldPos = world.xyz;
    vNormal = mat3(uPartModel) * aNormal;
    gl_Position = uPartViewProj * world;
}
)";

const char* kPartFragmentShaderSrc = R"(
#version 300 es
precision mediump float;
in vec3 vNormal;
in vec3 vWorldPos;
uniform vec3 uPartColor;
uniform vec3 uLightDir;
out vec4 fragColor;

void main() {
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);
    float diffuse = max(dot(normal, lightDir), 0.15);
    vec3 color = uPartColor * diffuse;
    fragColor = vec4(color, 1.0);
}
)";

}  // namespace

EngineRenderer::EngineRenderer() = default;

EngineRenderer::~EngineRenderer() {
    Stop();
    ClearSurface();
}

bool EngineRenderer::SetSurface(ANativeWindow* window) {
    std::scoped_lock lock(mutex_);
    if (window == window_) {
        return true;
    }

    ClearSurfaceLocked();

    if (!window) {
        return false;
    }

    window_ = window;
    ANativeWindow_acquire(window_);

    if (!egl_.Initialize(window_)) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Failed to initialize EGL context");
        ClearSurfaceLocked();
        return false;
    }

    width_ = egl_.Width();
    height_ = egl_.Height();
    camera_.SetViewport(width_, height_);

    InitializeGlResourcesLocked();
    return true;
}

void EngineRenderer::ClearSurface() {
    std::scoped_lock lock(mutex_);
    ClearSurfaceLocked();
}

void EngineRenderer::Resize(int width, int height) {
    std::scoped_lock lock(mutex_);
    width_ = width;
    height_ = height;
    camera_.SetViewport(width_, height_);
}

void EngineRenderer::Orbit(float deltaYaw, float deltaPitch) {
    std::scoped_lock lock(mutex_);
    camera_.Orbit(deltaYaw, deltaPitch);
}

void EngineRenderer::Pan(float deltaX, float deltaY) {
    std::scoped_lock lock(mutex_);
    camera_.Pan(deltaX, deltaY);
}

void EngineRenderer::Zoom(float scaleDelta) {
    std::scoped_lock lock(mutex_);
    camera_.Zoom(scaleDelta);
}

void EngineRenderer::SetPreferredFrameRate(int fps) {
    preferredFps_.store(fps);
}

void EngineRenderer::SetAssetManager(AAssetManager* assetManager) {
    std::scoped_lock lock(mutex_);
    assetManager_ = assetManager;
    assemblyLoaded_ = false;
}

void EngineRenderer::SetAssemblyMapping(const std::string& mappingPath) {
    std::scoped_lock lock(mutex_);
    assemblyMappingPath_ = mappingPath;
    assemblyLoaded_ = false;
}

void EngineRenderer::SetControlInputs(const EngineControlInputs& inputs) {
    std::scoped_lock lock(mutex_);
    controlInputs_ = inputs;
    physics_.SetControlInputs(inputs);
}

size_t EngineRenderer::PartCount() const {
    std::scoped_lock lock(mutex_);
    return assembly_.Parts().size();
}

bool EngineRenderer::CopyPartTransform(size_t index, Mat4* outMatrix, std::string* outName) const {
    std::scoped_lock lock(mutex_);
    const auto& parts = assembly_.Parts();
    if (index >= parts.size()) {
        return false;
    }
    if (outMatrix) {
        *outMatrix = parts[index].currentTransform;
    }
    if (outName) {
        *outName = parts[index].name;
    }
    return true;
}

void EngineRenderer::Start() {
    if (isRunning_) {
        return;
    }
    isRunning_ = true;
    lastFrameTime_.store(0);
    fps_.store(0.0f, std::memory_order_relaxed);
    frameTimeMs_.store(0.0f, std::memory_order_relaxed);
    frameCounter_.store(0, std::memory_order_relaxed);
    ScheduleNextFrame();
}

void EngineRenderer::Stop() {
    if (!isRunning_) {
        return;
    }
    isRunning_ = false;
    StopFallbackLoopLocked();
}

void EngineRenderer::InitializeGlResourcesLocked() {
    if (!egl_.IsValid() || !egl_.MakeCurrent()) {
        __android_log_print(ANDROID_LOG_WARN, kTag, "Cannot initialize GL resources without current context");
        return;
    }

    gridShader_.Destroy();
    partShader_.Destroy();
    gridPlane_.Destroy();
    assembly_.Destroy();
    assemblyLoaded_ = false;

    if (!gridShader_.Compile(kGridVertexShaderSrc, kGridFragmentShaderSrc)) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Failed to compile shader program");
        return;
    }

    uViewProj_ = glGetUniformLocation(gridShader_.Id(), "uViewProj");
    uModel_ = glGetUniformLocation(gridShader_.Id(), "uModel");
    uCameraPos_ = glGetUniformLocation(gridShader_.Id(), "uCameraPos");

    GLint extentLocation = glGetUniformLocation(gridShader_.Id(), "uExtent");
    GLint majorLocation = glGetUniformLocation(gridShader_.Id(), "uMajorStep");
    GLint minorLocation = glGetUniformLocation(gridShader_.Id(), "uMinorStep");

    gridPlane_.Initialize();

    glUseProgram(gridShader_.Id());
    glUniform1f(extentLocation, kPlaneExtent);
    glUniform1f(majorLocation, kMajorStep);
    glUniform1f(minorLocation, kMinorStep);
    glUseProgram(0);

    if (!partShader_.Compile(kPartVertexShaderSrc, kPartFragmentShaderSrc)) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Failed to compile part shader program");
        return;
    }

    uPartViewProj_ = glGetUniformLocation(partShader_.Id(), "uPartViewProj");
    uPartModel_ = glGetUniformLocation(partShader_.Id(), "uPartModel");
    uPartColor_ = glGetUniformLocation(partShader_.Id(), "uPartColor");
    uPartLightDir_ = glGetUniformLocation(partShader_.Id(), "uLightDir");

    EnsureAssemblyInitializedLocked();

    CopyGlString(glGetString(GL_RENDERER), gpuRenderer_);
    CopyGlString(glGetString(GL_VENDOR), gpuVendor_);
    CopyGlString(glGetString(GL_VERSION), gpuVersion_);

    egl_.DetachCurrent();
    eglReleaseThread();
}

void EngineRenderer::DestroyGlResourcesLocked() {
    if (!egl_.IsValid()) {
        gridShader_.Destroy();
        partShader_.Destroy();
        gridPlane_.Destroy();
        assembly_.Destroy();
        assemblyLoaded_ = false;
        return;
    }

    if (!egl_.MakeCurrent()) {
        gridShader_.Destroy();
        partShader_.Destroy();
        gridPlane_.Destroy();
        assembly_.Destroy();
        assemblyLoaded_ = false;
        return;
    }

    gridShader_.Destroy();
    partShader_.Destroy();
    gridPlane_.Destroy();
    assembly_.Destroy();
    assemblyLoaded_ = false;

    egl_.DetachCurrent();
    eglReleaseThread();
}

void EngineRenderer::ClearSurfaceLocked() {
    DestroyGlResourcesLocked();

    if (window_) {
        ANativeWindow_release(window_);
        window_ = nullptr;
    }
    width_ = 0;
    height_ = 0;
    egl_.Destroy();
}

void EngineRenderer::EnsureAssemblyInitializedLocked() {
    if (assemblyLoaded_) {
        return;
    }
    if (!assetManager_ || assemblyMappingPath_.empty()) {
        return;
    }

    assembly_.Destroy();
    if (!assembly_.Load(assetManager_, assemblyMappingPath_)) {
        __android_log_print(ANDROID_LOG_WARN, kTag, "Assembly load failed for '%s'", assemblyMappingPath_.c_str());
        assemblyLoaded_ = false;
        return;
    }

    physics_.SetAnchors(assembly_.Anchors());
    physics_.SetControlInputs(controlInputs_);
    assembly_.ApplyTransforms(physics_.Evaluate(0.0f));
    assemblyLoaded_ = true;
}

void EngineRenderer::RenderFrame(int64_t frameTimeNanos) {
    if (!isRunning_ || !egl_.IsValid()) {
        return;
    }

    std::scoped_lock lock(mutex_);

    eglBindAPI(EGL_OPENGL_ES_API);
    if (!egl_.MakeCurrent()) {
        return;
    }

    const int64_t previousTime = lastFrameTime_.exchange(frameTimeNanos, std::memory_order_relaxed);
    if (previousTime > 0) {
        const float deltaMs = static_cast<float>(frameTimeNanos - previousTime) / 1'000'000.0f;
        frameTimeMs_.store(deltaMs, std::memory_order_relaxed);
        if (deltaMs > 0.0f) {
            fps_.store(1000.0f / deltaMs, std::memory_order_relaxed);
        }
    }
    frameCounter_.fetch_add(1, std::memory_order_relaxed);

    glViewport(0, 0, width_, height_);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.04f, 0.05f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gridShader_.Id());

    const Mat4 model = Mat4::Identity();
    const Mat4 view = camera_.ViewMatrix();
    const Mat4 proj = camera_.ProjectionMatrix();
    const Mat4 viewProj = Multiply(proj, view);

    glUniformMatrix4fv(uViewProj_, 1, GL_FALSE, viewProj.Ptr());
    glUniformMatrix4fv(uModel_, 1, GL_FALSE, model.Ptr());

    const Vec3 eye = camera_.EyePosition();
    glUniform3f(uCameraPos_, eye.x, eye.y, eye.z);

    gridPlane_.Draw();

    float deltaSeconds = 0.0f;
    const float frameTime = frameTimeMs_.load(std::memory_order_relaxed);
    if (frameTime > 0.0f) {
        deltaSeconds = frameTime / 1000.0f;
    }

    EnsureAssemblyInitializedLocked();
    if (assemblyLoaded_) {
        const auto& transforms = physics_.Evaluate(deltaSeconds);
        assembly_.ApplyTransforms(transforms);

        glUseProgram(partShader_.Id());
        glUniformMatrix4fv(uPartViewProj_, 1, GL_FALSE, viewProj.Ptr());
        glUniform3f(uPartLightDir_, kLightDir[0], kLightDir[1], kLightDir[2]);

        const auto& parts = assembly_.Parts();
        for (const auto& part : parts) {
            glUniformMatrix4fv(uPartModel_, 1, GL_FALSE, part.currentTransform.Ptr());
            glUniform3f(uPartColor_, part.color.x, part.color.y, part.color.z);
            part.mesh.Draw();
        }
        glUseProgram(0);
    }

    egl_.SwapBuffers();
}

void EngineRenderer::FrameCallback(long frameTimeNanos, void* data) {
    auto* renderer = reinterpret_cast<EngineRenderer*>(data);
    if (!renderer) {
        return;
    }
    renderer->RenderFrame(frameTimeNanos);
    renderer->ScheduleNextFrame();
}

void EngineRenderer::ScheduleNextFrame() {
    if (!isRunning_) {
        return;
    }

#if __ANDROID_API__ >= 24
    AChoreographer* choreographer = AChoreographer_getInstance();
    if (choreographer) {
        AChoreographer_postFrameCallback64(choreographer, FrameCallback, this);
        return;
    }
#endif

    StartFallbackLoopLocked();
}

void EngineRenderer::StartFallbackLoopLocked() {
    bool expected = false;
    if (fallbackThreadRunning_.compare_exchange_strong(expected, true)) {
        fallbackThread_ = std::thread([this]() { FallbackLoop(); });
    }
}

void EngineRenderer::StopFallbackLoopLocked() {
    bool expected = true;
    if (fallbackThreadRunning_.compare_exchange_strong(expected, false)) {
        if (fallbackThread_.joinable()) {
            fallbackThread_.join();
        }
    }
}

void EngineRenderer::FallbackLoop() {
    using namespace std::chrono;
    auto interval = nanoseconds(1'000'000'000 / std::max(1, preferredFps_.load()));
    auto nextTick = steady_clock::now() + interval;

    while (fallbackThreadRunning_.load(std::memory_order_relaxed)) {
        if (!isRunning_) {
            std::this_thread::sleep_for(milliseconds(4));
            nextTick = steady_clock::now() + interval;
            continue;
        }

        const auto now = steady_clock::now();
        const auto nanos = duration_cast<nanoseconds>(now.time_since_epoch()).count();
        FrameCallback(static_cast<long>(nanos), this);

        interval = nanoseconds(1'000'000'000 / std::max(1, preferredFps_.load()));
        nextTick += interval;
        const auto sleepDuration = nextTick - steady_clock::now();
        if (sleepDuration > nanoseconds(0)) {
            std::this_thread::sleep_for(sleepDuration);
        } else {
            nextTick = steady_clock::now();
        }
    }
}

void EngineRenderer::FillDiagnostics(DiagnosticsSnapshot* outSnapshot) const {
    if (!outSnapshot) {
        return;
    }

    outSnapshot->fps = fps_.load(std::memory_order_relaxed);
    outSnapshot->frameTimeMs = frameTimeMs_.load(std::memory_order_relaxed);
    outSnapshot->frameCount = frameCounter_.load(std::memory_order_relaxed);
    outSnapshot->eglReady = egl_.IsValid();

    std::scoped_lock lock(mutex_);
    outSnapshot->surfaceWidth = width_;
    outSnapshot->surfaceHeight = height_;
    std::snprintf(outSnapshot->gpuRenderer, sizeof(outSnapshot->gpuRenderer), "%s", gpuRenderer_.data());
    std::snprintf(outSnapshot->gpuVendor, sizeof(outSnapshot->gpuVendor), "%s", gpuVendor_.data());
    std::snprintf(outSnapshot->gpuVersion, sizeof(outSnapshot->gpuVersion), "%s", gpuVersion_.data());
}

}  // namespace engine
