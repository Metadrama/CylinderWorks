package com.example.cylinderworks.engine

import android.content.Context
import android.graphics.PointF
import android.util.Log
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import io.flutter.plugin.platform.PlatformView
import io.flutter.FlutterInjector
import kotlin.math.ln

private enum class InteractionMode {
    NONE,
    ORBIT,
    PAN
}

class EngineRendererView(context: Context) : PlatformView, SurfaceHolder.Callback, EngineDiagnosticsRegistry.Provider {

    companion object {
        private const val TAG = "EngineRendererView"
    }

    private val surfaceView: SurfaceView
    private val scaleDetector: ScaleGestureDetector
    private var interactionMode = InteractionMode.NONE
    private var lastSingleTouch = PointF()
    private var panAnchor = PointF()

    private var rendererHandle: Long = 0

    init {
        rendererHandle = NativeBridge.nativeCreateRenderer()
        EngineDiagnosticsRegistry.register(this)

        if (rendererHandle != 0L) {
            NativeBridge.nativeSetAssetManager(rendererHandle, context.assets)
            val assetKey = FlutterInjector.instance().flutterLoader().getLookupKeyForAsset("assets/engine/assembly.json")
            val loaded = NativeBridge.nativeLoadAssembly(rendererHandle, assetKey)
            if (!loaded) {
                Log.w(TAG, "Failed to schedule assembly load for asset: $assetKey")
            }
        }

        surfaceView = object : SurfaceView(context) {
            override fun onTouchEvent(event: MotionEvent): Boolean {
                handleTouch(event)
                return true
            }
        }

        surfaceView.holder.addCallback(this)
        surfaceView.holder.setFormat(android.graphics.PixelFormat.OPAQUE)
        surfaceView.setZOrderOnTop(false)
        surfaceView.keepScreenOn = true

    scaleDetector = ScaleGestureDetector(context, object : ScaleGestureDetector.SimpleOnScaleGestureListener() {
            override fun onScale(detector: ScaleGestureDetector): Boolean {
                if (rendererHandle == 0L) return false
                val scaleFactor = detector.scaleFactor
                if (!scaleFactor.isNaN() && scaleFactor > 0f) {
                    // Natural log keeps zoom symmetric around 1.0.
                    NativeBridge.nativeZoom(rendererHandle, ln(scaleFactor.toDouble()).toFloat())
                    return true
                }
                return false
            }
        })

    }

    override fun getView(): View = surfaceView

    override fun dispose() {
        surfaceView.holder.removeCallback(this)
        NativeBridge.nativeStop(rendererHandle)
        NativeBridge.nativeClearSurface(rendererHandle)
        EngineDiagnosticsRegistry.unregister(this)
        NativeBridge.nativeDestroyRenderer(rendererHandle)
        rendererHandle = 0
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        if (rendererHandle == 0L) return
        val attached = NativeBridge.nativeSetSurface(rendererHandle, holder.surface)
        if (!attached) {
            Log.e(TAG, "Failed to attach renderer surface")
            return
        }
        NativeBridge.nativeSetPreferredFps(rendererHandle, 120)
        NativeBridge.nativeStart(rendererHandle)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        if (rendererHandle == 0L) return
        NativeBridge.nativeResize(rendererHandle, width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        if (rendererHandle == 0L) return
        NativeBridge.nativeStop(rendererHandle)
        NativeBridge.nativeClearSurface(rendererHandle)
    }

    override fun diagnostics(): Map<String, Any?>? {
        if (rendererHandle == 0L) return null
        return NativeBridge.nativeGetDiagnostics(rendererHandle)?.toMutableMap()
    }

    private fun handleTouch(event: MotionEvent) {
        scaleDetector.onTouchEvent(event)

        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                interactionMode = InteractionMode.ORBIT
                lastSingleTouch.set(event.x, event.y)
            }

            MotionEvent.ACTION_POINTER_DOWN -> {
                if (event.pointerCount >= 2) {
                    interactionMode = InteractionMode.PAN
                    panAnchor = computeCentroid(event)
                }
            }

            MotionEvent.ACTION_MOVE -> {
                if (rendererHandle == 0L) return

                when (interactionMode) {
                    InteractionMode.ORBIT -> {
                        val dx = event.x - lastSingleTouch.x
                        val dy = event.y - lastSingleTouch.y
                        NativeBridge.nativeOrbit(rendererHandle, -dx, -dy)
                        lastSingleTouch.set(event.x, event.y)
                    }
                    InteractionMode.PAN -> {
                        if (event.pointerCount >= 2 && !scaleDetector.isInProgress) {
                            val centroid = computeCentroid(event)
                            val dx = centroid.x - panAnchor.x
                            val dy = centroid.y - panAnchor.y
                            NativeBridge.nativePan(rendererHandle, dx, dy)
                            panAnchor = centroid
                        }
                    }
                    else -> Unit
                }
            }

            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                interactionMode = InteractionMode.NONE
            }

            MotionEvent.ACTION_POINTER_UP -> {
                if (event.pointerCount - 1 <= 1) {
                    interactionMode = InteractionMode.ORBIT
                    val index = if (event.actionIndex == 0) 1 else 0
                    if (index < event.pointerCount) {
                        lastSingleTouch.set(event.getX(index), event.getY(index))
                    }
                }
            }
        }
    }

    private fun computeCentroid(event: MotionEvent): PointF {
        var sumX = 0f
        var sumY = 0f
        val count = event.pointerCount.coerceAtLeast(1)
        for (i in 0 until count) {
            sumX += event.getX(i)
            sumY += event.getY(i)
        }
        return PointF(sumX / count, sumY / count)
    }
}
