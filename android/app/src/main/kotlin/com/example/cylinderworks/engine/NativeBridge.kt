package com.example.cylinderworks.engine

import android.content.res.AssetManager

object NativeBridge {
    init {
        System.loadLibrary("engine_renderer")
    }

    external fun nativeCreateRenderer(): Long
    external fun nativeDestroyRenderer(handle: Long)
    external fun nativeSetSurface(handle: Long, surface: android.view.Surface?): Boolean
    external fun nativeSetAssetManager(handle: Long, assetManager: AssetManager)
    external fun nativeLoadAssembly(handle: Long, assetKey: String): Boolean
    external fun nativeSetControlInputs(handle: Long, throttle: Float, starter: Boolean, ignition: Boolean)
    external fun nativeClearSurface(handle: Long)
    external fun nativeResize(handle: Long, width: Int, height: Int)
    external fun nativeStart(handle: Long)
    external fun nativeStop(handle: Long)
    external fun nativeOrbit(handle: Long, dx: Float, dy: Float)
    external fun nativePan(handle: Long, dx: Float, dy: Float)
    external fun nativeZoom(handle: Long, delta: Float)
    external fun nativeSetPreferredFps(handle: Long, fps: Int)
    external fun nativeGetDiagnostics(handle: Long): Map<String, Any?>?
}
