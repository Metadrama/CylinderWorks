#pragma once

#include <EGL/egl.h>
#include <android/native_window.h>
#include <cstdint>

namespace engine {

class EglContext {
public:
    EglContext() = default;
    ~EglContext();

    bool Initialize(ANativeWindow* window);
    void Destroy();

    bool IsValid() const { return display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE && context_ != EGL_NO_CONTEXT; }

    bool MakeCurrent();
    bool DetachCurrent();
    void SwapBuffers();
    void SetPresentationTime(int64_t nanoseconds);

    int Width() const { return width_; }
    int Height() const { return height_; }

private:
    bool CreateSurface(ANativeWindow* window);
    void ReleaseSurface();

    EGLDisplay display_{EGL_NO_DISPLAY};
    EGLSurface surface_{EGL_NO_SURFACE};
    EGLContext context_{EGL_NO_CONTEXT};
    EGLConfig config_{nullptr};

    int width_{0};
    int height_{0};
};

}  // namespace engine
