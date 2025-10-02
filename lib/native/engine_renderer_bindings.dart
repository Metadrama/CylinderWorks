import 'dart:ffi' as ffi;
import 'dart:io' show Platform;

typedef _CreateNative = ffi.Int64 Function();
typedef _CreateDart = int Function();
typedef _DestroyNative = ffi.Void Function(ffi.Int64);
typedef _DestroyDart = void Function(int);
typedef _ResizeNative = ffi.Void Function(ffi.Int64, ffi.Int32, ffi.Int32);
typedef _ResizeDart = void Function(int, int, int);
typedef _OrbitNative = ffi.Void Function(ffi.Int64, ffi.Float, ffi.Float);
typedef _OrbitDart = void Function(int, double, double);
typedef _PanNative = ffi.Void Function(ffi.Int64, ffi.Float, ffi.Float);
typedef _PanDart = void Function(int, double, double);
typedef _ZoomNative = ffi.Void Function(ffi.Int64, ffi.Float);
typedef _ZoomDart = void Function(int, double);
typedef _FpsNative = ffi.Void Function(ffi.Int64, ffi.Int32);
typedef _FpsDart = void Function(int, int);
typedef _StartNative = ffi.Void Function(ffi.Int64);
typedef _StartDart = void Function(int);
typedef _StopNative = ffi.Void Function(ffi.Int64);
typedef _StopDart = void Function(int);

/// Thin wrapper around the native renderer library for use via FFI.
/// The Android platform view already talks to these entrypoints through JNI,
/// but exposing them here keeps the Flutter API explicit and portable.
class EngineRendererBindings {
  EngineRendererBindings._();

  static final EngineRendererBindings instance = EngineRendererBindings._();

  ffi.DynamicLibrary? _library;

  _CreateDart? _create;
  _DestroyDart? _destroy;
  _ResizeDart? _resize;
  _OrbitDart? _orbit;
  _PanDart? _pan;
  _ZoomDart? _zoom;
  _FpsDart? _setFps;
  _StartDart? _start;
  _StopDart? _stop;

  bool get isLoaded => _library != null;

  void loadIfNeeded() {
    if (_library != null) {
      return;
    }

    final libName = _resolveLibraryName();
    if (libName == null) {
      return;
    }

    try {
      _library = ffi.DynamicLibrary.open(libName);
  _create = _library!.lookupFunction<_CreateNative, _CreateDart>('engine_renderer_create');
  _destroy = _library!.lookupFunction<_DestroyNative, _DestroyDart>('engine_renderer_destroy');
  _resize = _library!.lookupFunction<_ResizeNative, _ResizeDart>('engine_renderer_resize');
  _orbit = _library!.lookupFunction<_OrbitNative, _OrbitDart>('engine_renderer_orbit');
  _pan = _library!.lookupFunction<_PanNative, _PanDart>('engine_renderer_pan');
  _zoom = _library!.lookupFunction<_ZoomNative, _ZoomDart>('engine_renderer_zoom');
  _setFps = _library!.lookupFunction<_FpsNative, _FpsDart>('engine_renderer_set_preferred_fps');
  _start = _library!.lookupFunction<_StartNative, _StartDart>('engine_renderer_start');
  _stop = _library!.lookupFunction<_StopNative, _StopDart>('engine_renderer_stop');
    } on Object {
      // FFI is optional on platforms where we cannot load a native library yet.
      _library = null;
    }
  }

  int create() {
    loadIfNeeded();
    return _create?.call() ?? 0;
  }

  void destroy(int handle) {
    _destroy?.call(handle);
  }

  void resize(int handle, int width, int height) {
    _resize?.call(handle, width, height);
  }

  void orbit(int handle, double dx, double dy) {
    _orbit?.call(handle, dx, dy);
  }

  void pan(int handle, double dx, double dy) {
    _pan?.call(handle, dx, dy);
  }

  void zoom(int handle, double delta) {
    _zoom?.call(handle, delta);
  }

  void setPreferredFps(int handle, int fps) {
    _setFps?.call(handle, fps);
  }

  void start(int handle) {
    _start?.call(handle);
  }

  void stop(int handle) {
    _stop?.call(handle);
  }

  String? _resolveLibraryName() {
    if (Platform.isAndroid) {
      return 'libengine_renderer.so';
    }
    if (Platform.isLinux || Platform.isWindows || Platform.isMacOS) {
      return null; // TODO: add desktop builds when ready.
    }
    if (Platform.isIOS) {
      return null;
    }
    return null;
  }
}
