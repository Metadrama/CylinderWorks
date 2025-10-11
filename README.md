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
- Wire the generated Simscape assembly export (parts + constraints) into the runtime kinematics layer.
- Bridge structured telemetry (RPM, torque, cycle phase) from native physics once the solver is ready.
- Expand the platform support matrix (desktop, iOS) using the same FFI façade.

## UI/UX Design System

CylinderWorks uses a professional design system built on Material 3 with custom extensions for mechanical engineering applications. The design system provides:

- **Consistent spacing** using an 8-point grid system
- **Professional color palette** optimized for dark mode and data visualization
- **Reusable components** for metrics, panels, and controls
- **Mobile-first responsive design** with proper touch targets
- **Engineering-focused features** like monospace fonts for numeric data

### Documentation

- [UI Design System Study](docs/UI_DESIGN_SYSTEM_STUDY.md) - Comprehensive overview of design decisions and options
- [UI Implementation Guide](docs/UI_IMPLEMENTATION_GUIDE.md) - Step-by-step guide for developers
- [Before & After Comparison](docs/UI_BEFORE_AFTER_COMPARISON.md) - Visual comparison showing improvements

### Quick Start

```dart
// Use design tokens for consistent spacing
import 'package:cylinderworks/theme/design_tokens.dart';

Padding(
  padding: const EdgeInsets.all(DesignTokens.spaceMd),
  child: child,
)

// Use reusable components
import 'package:cylinderworks/ui/components/metric_tile.dart';

MetricTile(
  title: 'Engine Speed',
  value: '3450',
  unit: 'RPM',
  color: AppColors.dataRpm,
)
```

