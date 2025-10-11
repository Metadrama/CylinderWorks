import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../native/engine_renderer_bindings.dart';
import '../theme/design_tokens.dart';
import 'components/info_line.dart';
import 'components/info_panel.dart';
import 'components/metric_tile.dart';

const _kDiagnosticsChannel = MethodChannel('engine/diagnostics');

class DiagnosticsSnapshot {
  DiagnosticsSnapshot({
    this.fps,
    this.frameTimeMs,
    this.surfaceWidth,
    this.surfaceHeight,
    this.frameCount,
    this.partCount,
    this.constraintCount,
    this.gpuRenderer,
    this.gpuVendor,
    this.gpuVersion,
    this.cpuHardware,
    this.socModel,
    this.socManufacturer,
    this.deviceModel,
    this.deviceManufacturer,
    this.eglReady,
  });

  final double? fps;
  final double? frameTimeMs;
  final int? surfaceWidth;
  final int? surfaceHeight;
  final int? frameCount;
  final int? partCount;
  final int? constraintCount;
  final String? gpuRenderer;
  final String? gpuVendor;
  final String? gpuVersion;
  final String? cpuHardware;
  final String? socModel;
  final String? socManufacturer;
  final String? deviceModel;
  final String? deviceManufacturer;
  final bool? eglReady;

  String get renderingLabel {
  final String gpu = (gpuRenderer?.isNotEmpty ?? false) ? gpuRenderer! : 'Unknown GPU';
  final String? cpu = (socModel?.isNotEmpty ?? false)
    ? socModel
    : (cpuHardware?.isNotEmpty ?? false)
      ? cpuHardware
      : deviceModel;
  if (cpu == null || cpu.isEmpty) {
    return gpu;
    }
  return '$gpu · $cpu';
  }

  String get fpsLabel {
    if (fps == null || fps!.isNaN) {
      return '-- fps';
    }
    return '${fps!.toStringAsFixed(1)} fps';
  }

  String get frameTimeLabel {
    if (frameTimeMs == null || frameTimeMs!.isNaN) {
      return '-- ms/frame';
    }
    return '${frameTimeMs!.toStringAsFixed(2)} ms/frame';
  }

  String? get frameCountLabel => frameCount?.toString();

  DiagnosticsSnapshot merge(DiagnosticsSnapshot other) {
    return DiagnosticsSnapshot(
      fps: other.fps ?? fps,
      frameTimeMs: other.frameTimeMs ?? frameTimeMs,
      surfaceWidth: other.surfaceWidth ?? surfaceWidth,
      surfaceHeight: other.surfaceHeight ?? surfaceHeight,
      frameCount: other.frameCount ?? frameCount,
      partCount: other.partCount ?? partCount,
      constraintCount: other.constraintCount ?? constraintCount,
      gpuRenderer: other.gpuRenderer ?? gpuRenderer,
      gpuVendor: other.gpuVendor ?? gpuVendor,
      gpuVersion: other.gpuVersion ?? gpuVersion,
      cpuHardware: other.cpuHardware ?? cpuHardware,
      socModel: other.socModel ?? socModel,
      socManufacturer: other.socManufacturer ?? socManufacturer,
      deviceModel: other.deviceModel ?? deviceModel,
      deviceManufacturer: other.deviceManufacturer ?? deviceManufacturer,
      eglReady: other.eglReady ?? eglReady,
    );
  }

  static DiagnosticsSnapshot fromMap(Map<Object?, Object?>? map) {
    if (map == null) {
      return DiagnosticsSnapshot();
    }

    T? _cast<T>(Object? value) {
      if (value is T) {
        return value;
      }
      return null;
    }

    double? _asDouble(Object? value) {
      if (value is num) {
        return value.toDouble();
      }
      return null;
    }

    int? _asInt(Object? value) {
      if (value is num) {
        return value.toInt();
      }
      return null;
    }

    return DiagnosticsSnapshot(
      fps: _asDouble(map['fps']),
      frameTimeMs: _asDouble(map['frameTimeMs']),
      surfaceWidth: _asInt(map['surfaceWidth']),
      surfaceHeight: _asInt(map['surfaceHeight']),
      frameCount: _asInt(map['frameCount']),
      partCount: _asInt(map['partCount']),
      constraintCount: _asInt(map['constraintCount']),
      gpuRenderer: _cast<String>(map['gpuRenderer']),
      gpuVendor: _cast<String>(map['gpuVendor']),
      gpuVersion: _cast<String>(map['gpuVersion']),
      cpuHardware: _cast<String>(map['cpuHardware']),
      socModel: _cast<String>(map['socModel']),
      socManufacturer: _cast<String>(map['socManufacturer']),
      deviceModel: _cast<String>(map['deviceModel']),
      deviceManufacturer: _cast<String>(map['deviceManufacturer']),
      eglReady: _cast<bool>(map['eglReady']),
    );
  }
}

class MonitoringOverlay extends StatefulWidget {
  const MonitoringOverlay({super.key, required this.onClose});

  final VoidCallback onClose;

  @override
  State<MonitoringOverlay> createState() => _MonitoringOverlayState();
}

class _MonitoringOverlayState extends State<MonitoringOverlay> {
  DiagnosticsSnapshot _snapshot = DiagnosticsSnapshot();
  Timer? _timer;
  bool _hasChannel = true;

  @override
  void initState() {
    super.initState();
    _pollDiagnostics();
    _timer = Timer.periodic(const Duration(seconds: 1), (_) => _pollDiagnostics());
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  Future<void> _pollDiagnostics() async {
    try {
      final map = await _kDiagnosticsChannel.invokeMapMethod<Object?, Object?>('getSnapshot');
      if (!mounted) return;
      final incoming = DiagnosticsSnapshot.fromMap(map);
      setState(() {
        _snapshot = _snapshot.merge(incoming);
        _hasChannel = true;
      });
    } on MissingPluginException {
      if (!mounted) return;
      setState(() {
        _hasChannel = false;
      });
    } catch (_) {
      // Leave previous snapshot untouched to avoid flickering.
    }
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final bool ffiEnabled = EngineRendererBindings.instance.isLoaded;

    final surfaceLabel = _snapshot.surfaceWidth != null && _snapshot.surfaceHeight != null
        ? '${_snapshot.surfaceWidth}×${_snapshot.surfaceHeight}'
        : null;
    final vendorLabel = (_snapshot.gpuVendor?.isNotEmpty ?? false) ? _snapshot.gpuVendor : null;
    final deviceParts = <String>[];
    if (_snapshot.deviceManufacturer?.isNotEmpty ?? false) {
      deviceParts.add(_snapshot.deviceManufacturer!);
    }
    if (_snapshot.deviceModel?.isNotEmpty ?? false) {
      deviceParts.add(_snapshot.deviceModel!);
    }
    final deviceLabel = deviceParts.isNotEmpty ? deviceParts.join(' ') : null;

    return InfoPanel(
      title: 'Realtime Monitor',
      icon: Icons.speed,
      onClose: widget.onClose,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          InfoLine(label: 'Rendering', value: _snapshot.renderingLabel),
          if (_snapshot.gpuVersion != null && _snapshot.gpuVersion!.isNotEmpty)
            InfoLine(label: 'GL', value: _snapshot.gpuVersion!),
          if (vendorLabel != null)
            InfoLine(label: 'Vendor', value: vendorLabel),
          if (deviceLabel != null)
            InfoLine(label: 'Device', value: deviceLabel),
          if (_hasChannel)
            InfoLine(
              label: 'Pipeline',
              value: _snapshot.eglReady == true ? 'EGL · active' : 'EGL · standby',
            )
          else
            const InfoLine(label: 'Pipeline', value: 'Channel unavailable'),
          InfoLine(
            label: 'FFI/Ffigen',
            value: ffiEnabled ? 'enabled' : 'disabled',
          ),
          const SizedBox(height: DesignTokens.spaceSm),
          Row(
            children: [
              Expanded(
                child: MetricTile(
                  title: 'FPS',
                  value: _snapshot.fpsLabel,
                  color: theme.colorScheme.secondary,
                ),
              ),
              const SizedBox(width: DesignTokens.spaceSm),
              Expanded(
                child: MetricTile(
                  title: 'Frame',
                  value: _snapshot.frameTimeLabel,
                  color: theme.colorScheme.tertiary,
                ),
              ),
            ],
          ),
          if (surfaceLabel != null) ...[
            const SizedBox(height: DesignTokens.spaceSm),
            InfoLine(label: 'Viewport', value: surfaceLabel),
          ],
          if (_snapshot.frameCountLabel != null) ...[
            const SizedBox(height: DesignTokens.spaceSm),
            InfoLine(label: 'Frames', value: _snapshot.frameCountLabel!),
          ],
          if (_snapshot.partCount != null || _snapshot.constraintCount != null) ...[
            const SizedBox(height: DesignTokens.spaceSm),
            if (_snapshot.partCount != null)
              InfoLine(label: 'Parts', value: _snapshot.partCount!.toString()),
            if (_snapshot.constraintCount != null)
              InfoLine(label: 'Constraints', value: _snapshot.constraintCount!.toString()),
          ],
        ],
      ),
    );
  }
}
