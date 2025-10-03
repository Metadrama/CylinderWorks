#include "engine/platform/android/egl_context.h"

#include <EGL/eglext.h>
#include <android/log.h>

namespace engine {

namespace {
constexpr const char* kTag = "EngineRenderer";

void LogEglError(const char* message) {
    const EGLint error = eglGetError();
    __android_log_print(ANDROID_LOG_ERROR, kTag, "%s (0x%x)", message, error);
}

EGLConfig ChooseConfig(EGLDisplay display) {
    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };

    EGLConfig config = nullptr;
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs) || numConfigs <= 0) {
        LogEglError("eglChooseConfig failed");
        return nullptr;
    }
    return config;
}

}  // namespace

EglContext::~EglContext() {
    Destroy();
}

bool EglContext::Initialize(ANativeWindow* window) {
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display_ == EGL_NO_DISPLAY) {
        LogEglError("Failed to get EGL display");
        return false;
    }

    if (!eglInitialize(display_, nullptr, nullptr)) {
        LogEglError("Failed to initialize EGL");
        Destroy();
        return false;
    }

    config_ = ChooseConfig(display_);
    if (!config_) {
        Destroy();
        return false;
    }

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    context_ = eglCreateContext(display_, config_, EGL_NO_CONTEXT, contextAttribs);
    if (context_ == EGL_NO_CONTEXT) {
        LogEglError("Failed to create EGL context");
        Destroy();
        return false;
    }

    if (!CreateSurface(window)) {
        Destroy();
        return false;
    }

    if (!MakeCurrent()) {
        Destroy();
        return false;
    }

    return true;
}

bool EglContext::CreateSurface(ANativeWindow* window) {
    ReleaseSurface();

    if (!window) {
        __android_log_print(ANDROID_LOG_WARN, kTag, "CreateSurface called with null window");
        return false;
    }

    surface_ = eglCreateWindowSurface(display_, config_, window, nullptr);
    if (surface_ == EGL_NO_SURFACE) {
        LogEglError("Failed to create window surface");
        return false;
    }

    if (!eglQuerySurface(display_, surface_, EGL_WIDTH, &width_)) {
        LogEglError("Failed to query surface width");
    }
    if (!eglQuerySurface(display_, surface_, EGL_HEIGHT, &height_)) {
        LogEglError("Failed to query surface height");
    }

    return true;
}

bool EglContext::MakeCurrent() {
    if (display_ == EGL_NO_DISPLAY || surface_ == EGL_NO_SURFACE || context_ == EGL_NO_CONTEXT) {
        return false;
    }
    if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
        LogEglError("eglMakeCurrent failed");
        return false;
    }
    return true;
}

bool EglContext::DetachCurrent() {
    if (display_ == EGL_NO_DISPLAY) {
        return false;
    }
    if (!eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        LogEglError("eglMakeCurrent (detach) failed");
        return false;
    }
    return true;
}

void EglContext::SwapBuffers() {
    if (display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE) {
        eglSwapBuffers(display_, surface_);
    }
}

void EglContext::SetPresentationTime(int64_t nanoseconds) {
#ifdef EGL_ANDROID_presentation_time
    if (surface_ == EGL_NO_SURFACE || display_ == EGL_NO_DISPLAY) {
        return;
    }

    static const auto presentationTimeFn = reinterpret_cast<PFNEGLPRESENTATIONTIMEANDROIDPROC>(
        eglGetProcAddress("eglPresentationTimeANDROID"));

    if (presentationTimeFn) {
        presentationTimeFn(display_, surface_, nanoseconds);
    }
#else
    (void)nanoseconds;
#endif
}

void EglContext::ReleaseSurface() {
    if (display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE) {
        eglDestroySurface(display_, surface_);
        surface_ = EGL_NO_SURFACE;
        width_ = 0;
        height_ = 0;
    }
}

void EglContext::Destroy() {
    ReleaseSurface();

    if (display_ != EGL_NO_DISPLAY && context_ != EGL_NO_CONTEXT) {
        eglDestroyContext(display_, context_);
        context_ = EGL_NO_CONTEXT;
    }

    if (display_ != EGL_NO_DISPLAY) {
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}

}  // namespace engine
