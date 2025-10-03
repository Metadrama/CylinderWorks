#pragma once

#include <cstdint>

namespace engine {

struct DiagnosticsSnapshot {
    float fps{0.0f};
    float frameTimeMs{0.0f};
    int32_t surfaceWidth{0};
    int32_t surfaceHeight{0};
    int32_t frameCount{0};
    bool eglReady{false};
    char gpuRenderer[128] = {0};
    char gpuVendor[128] = {0};
    char gpuVersion[128] = {0};
};

}  // namespace engine
