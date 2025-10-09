#include <jni.h>

#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <new>
#include <algorithm>
#include <cstdint>
#include <string>
#include <cstring>

#include "engine/platform/android/engine_renderer.h"

namespace {
constexpr const char* kTag = "EngineRenderer";

inline engine::EngineRenderer* FromHandle(jlong handle) {
    return reinterpret_cast<engine::EngineRenderer*>(handle);
}

inline engine::EngineRenderer* FromPointer(int64_t ptr) {
    return reinterpret_cast<engine::EngineRenderer*>(ptr);
}

template <typename T>
inline int64_t ToPointer(T* ptr) {
    return reinterpret_cast<int64_t>(ptr);
}

std::string ToStdString(JNIEnv* env, jstring value) {
    if (!env || !value) {
        return {};
    }
    const char* chars = env->GetStringUTFChars(value, nullptr);
    if (!chars) {
        return {};
    }
    std::string result(chars);
    env->ReleaseStringUTFChars(value, chars);
    return result;
}

}  // namespace

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeCreateRenderer(JNIEnv* env, jclass /*clazz*/) {
    auto* renderer = new (std::nothrow) engine::EngineRenderer();
    if (!renderer) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Failed to allocate EngineRenderer");
        return 0;
    }
    return reinterpret_cast<jlong>(renderer);
}

JNIEXPORT jobject JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeGetDiagnostics(JNIEnv* env, jclass /*clazz*/, jlong handle) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return nullptr;
    }

    engine::DiagnosticsSnapshot snapshot{};
    renderer->FillDiagnostics(&snapshot);

    jclass hashMapClass = env->FindClass("java/util/HashMap");
    if (!hashMapClass) {
        return nullptr;
    }
    jmethodID hashMapCtor = env->GetMethodID(hashMapClass, "<init>", "()V");
    jmethodID putMethod = env->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    if (!hashMapCtor || !putMethod) {
        return nullptr;
    }
    jobject map = env->NewObject(hashMapClass, hashMapCtor);
    if (!map) {
        env->DeleteLocalRef(hashMapClass);
        return nullptr;
    }

    jclass doubleClass = env->FindClass("java/lang/Double");
    jclass integerClass = env->FindClass("java/lang/Integer");
    jclass booleanClass = env->FindClass("java/lang/Boolean");
    if (!doubleClass || !integerClass || !booleanClass) {
        if (doubleClass) {
            env->DeleteLocalRef(doubleClass);
        }
        if (integerClass) {
            env->DeleteLocalRef(integerClass);
        }
        if (booleanClass) {
            env->DeleteLocalRef(booleanClass);
        }
        return map;
    }
    jmethodID doubleValueOf = env->GetStaticMethodID(doubleClass, "valueOf", "(D)Ljava/lang/Double;");
    jmethodID integerValueOf = env->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;");
    jmethodID booleanValueOf = env->GetStaticMethodID(booleanClass, "valueOf", "(Z)Ljava/lang/Boolean;");

    auto putString = [&](const char* key, const char* value) {
        jstring jKey = env->NewStringUTF(key);
        jstring jValue = env->NewStringUTF(value ? value : "");
        env->CallObjectMethod(map, putMethod, jKey, jValue);
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(jValue);
    };

    auto putDouble = [&](const char* key, double value) {
        if (!doubleValueOf) return;
        jstring jKey = env->NewStringUTF(key);
        jobject jValue = env->CallStaticObjectMethod(doubleClass, doubleValueOf, static_cast<jdouble>(value));
        env->CallObjectMethod(map, putMethod, jKey, jValue);
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(jValue);
    };

    auto putInt = [&](const char* key, jint value) {
        if (!integerValueOf) return;
        jstring jKey = env->NewStringUTF(key);
        jobject jValue = env->CallStaticObjectMethod(integerClass, integerValueOf, value);
        env->CallObjectMethod(map, putMethod, jKey, jValue);
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(jValue);
    };

    auto putBool = [&](const char* key, jboolean value) {
        if (!booleanValueOf) return;
        jstring jKey = env->NewStringUTF(key);
        jobject jValue = env->CallStaticObjectMethod(booleanClass, booleanValueOf, value);
        env->CallObjectMethod(map, putMethod, jKey, jValue);
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(jValue);
    };

    putDouble("fps", snapshot.fps);
    putDouble("frameTimeMs", snapshot.frameTimeMs);
    putInt("surfaceWidth", snapshot.surfaceWidth);
    putInt("surfaceHeight", snapshot.surfaceHeight);
    putInt("frameCount", snapshot.frameCount);
    putBool("eglReady", snapshot.eglReady ? JNI_TRUE : JNI_FALSE);
    putString("gpuRenderer", snapshot.gpuRenderer);
    putString("gpuVendor", snapshot.gpuVendor);
    putString("gpuVersion", snapshot.gpuVersion);

    env->DeleteLocalRef(hashMapClass);
    if (doubleClass) {
        env->DeleteLocalRef(doubleClass);
    }
    if (integerClass) {
        env->DeleteLocalRef(integerClass);
    }
    if (booleanClass) {
        env->DeleteLocalRef(booleanClass);
    }

    return map;
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeDestroyRenderer(JNIEnv* env, jclass /*clazz*/, jlong handle) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    renderer->Stop();
    renderer->ClearSurface();
    delete renderer;
}

JNIEXPORT jboolean JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeSetSurface(JNIEnv* env, jclass /*clazz*/, jlong handle, jobject surface) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return JNI_FALSE;
    }

    ANativeWindow* window = nullptr;
    if (surface) {
        window = ANativeWindow_fromSurface(env, surface);
    }

    const bool result = renderer->SetSurface(window);
    if (window) {
        ANativeWindow_release(window);
    }

    return result ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeSetAssetManager(JNIEnv* env, jclass /*clazz*/, jlong handle, jobject assetManager) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    AAssetManager* manager = AAssetManager_fromJava(env, assetManager);
    renderer->SetAssetManager(manager);
}

JNIEXPORT jboolean JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeLoadAssembly(JNIEnv* env, jclass /*clazz*/, jlong handle, jstring assetKey) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return JNI_FALSE;
    }
    std::string key = ToStdString(env, assetKey);
    renderer->SetAssemblyMapping(key);
    return key.empty() ? JNI_FALSE : JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeResize(JNIEnv* env, jclass /*clazz*/, jlong handle, jint width, jint height) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    renderer->Resize(width, height);
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeStart(JNIEnv* env, jclass /*clazz*/, jlong handle) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    renderer->Start();
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeStop(JNIEnv* env, jclass /*clazz*/, jlong handle) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    renderer->Stop();
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeOrbit(JNIEnv* env, jclass /*clazz*/, jlong handle, jfloat dx, jfloat dy) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    renderer->Orbit(dx, dy);
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativePan(JNIEnv* env, jclass /*clazz*/, jlong handle, jfloat dx, jfloat dy) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    renderer->Pan(dx, dy);
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeZoom(JNIEnv* env, jclass /*clazz*/, jlong handle, jfloat delta) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    renderer->Zoom(delta);
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeSetPreferredFps(JNIEnv* env, jclass /*clazz*/, jlong handle, jint fps) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    renderer->SetPreferredFrameRate(fps);
}

JNIEXPORT void JNICALL
Java_com_example_cylinderworks_engine_NativeBridge_nativeClearSurface(JNIEnv* env, jclass /*clazz*/, jlong handle) {
    auto* renderer = FromHandle(handle);
    if (!renderer) {
        return;
    }
    renderer->ClearSurface();
}

}  // extern "C"

extern "C" {

int64_t engine_renderer_create() {
    auto* renderer = new (std::nothrow) engine::EngineRenderer();
    return ToPointer(renderer);
}

void engine_renderer_destroy(int64_t handle) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return;
    }
    renderer->Stop();
    renderer->ClearSurface();
    delete renderer;
}

void engine_renderer_resize(int64_t handle, int width, int height) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return;
    }
    renderer->Resize(width, height);
}

void engine_renderer_orbit(int64_t handle, float dx, float dy) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return;
    }
    renderer->Orbit(dx, dy);
}

void engine_renderer_pan(int64_t handle, float dx, float dy) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return;
    }
    renderer->Pan(dx, dy);
}

void engine_renderer_zoom(int64_t handle, float delta) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return;
    }
    renderer->Zoom(delta);
}

void engine_renderer_set_preferred_fps(int64_t handle, int fps) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return;
    }
    renderer->SetPreferredFrameRate(fps);
}

void engine_renderer_start(int64_t handle) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return;
    }
    renderer->Start();
}

void engine_renderer_stop(int64_t handle) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return;
    }
    renderer->Stop();
}

int32_t engine_renderer_part_count(int64_t handle) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return 0;
    }
    return static_cast<int32_t>(renderer->PartCount());
}

int engine_renderer_copy_part_transform(int64_t handle, int32_t index, float* outMatrix16, char* nameBuffer, size_t nameBufferLength) {
    auto* renderer = FromPointer(handle);
    if (!renderer) {
        return 0;
    }
    engine::Mat4 matrix;
    std::string name;
    if (!renderer->CopyPartTransform(static_cast<size_t>(index), outMatrix16 ? &matrix : nullptr, nameBuffer ? &name : nullptr)) {
        return 0;
    }
    if (outMatrix16) {
        std::memcpy(outMatrix16, matrix.data.data(), sizeof(float) * 16);
    }
    if (nameBuffer && nameBufferLength > 0) {
        const size_t copy = std::min(name.size(), nameBufferLength - 1);
        std::memcpy(nameBuffer, name.data(), copy);
        nameBuffer[copy] = '\0';
    }
    return 1;
}

}  // extern "C"
