# cylinderworks

Flutter-based engine simulator prototype with a native 3D rendering baseline.

## Current milestone – Native renderer baseline

- Embedded Android `SurfaceView` powered by a C++ OpenGL ES 3.0 renderer.
- Renders a performant infinite-style ground grid with orbit / pan / zoom camera controls.
- Renders at the device refresh rate via `AChoreographer` (falls back to a steady worker thread for older APIs).
- Exposes a thin `dart:ffi` surface (`EngineRendererBindings`) so Flutter can issue viewport and input commands directly.
- Keeps Flutter-side integration minimal: a single `EngineRendererView` widget hosts the native surface.
- Adds a dismissable diagnostics overlay (FPS, backend, device) fed by native-side instrumentation.

## Building

```bash
flutter pub get
flutter run -d android
```

> ✅ The native library is currently built for `arm64-v8a` and `armeabi-v7a` Android targets. Desktop/iOS hooks will land in later milestones.

## Controls

- One finger drag → orbit camera.
- Two finger drag → pan target.
- Pinch → zoom.

## Next steps

- Enrich the renderer with engine component meshes.
- Bridge structured telemetry (RPM, torque, cycle phase) from native physics once the solver is ready.
- Expand the platform support matrix (desktop, iOS) using the same FFI façade.
