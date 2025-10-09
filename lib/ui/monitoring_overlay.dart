import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../native/engine_renderer_bindings.dart';

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

    return Card(
      color: theme.colorScheme.surface.withOpacity(0.9),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(14)),
      elevation: 6,
      child: ConstrainedBox(
        constraints: const BoxConstraints(minWidth: 240, maxWidth: 320),
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                children: [
                  const Icon(Icons.speed, size: 18),
                  const SizedBox(width: 8),
                  const Expanded(
                    child: Text(
                      'Realtime Monitor',
                      style: TextStyle(fontWeight: FontWeight.w600),
                    ),
                  ),
                  IconButton(
                    visualDensity: VisualDensity.compact,
                    padding: EdgeInsets.zero,
                    constraints: const BoxConstraints(),
                    icon: const Icon(Icons.close, size: 16),
                    onPressed: widget.onClose,
                  ),
                ],
              ),
              const SizedBox(height: 8),
              _InfoLine(label: 'Rendering', value: _snapshot.renderingLabel),
              if (_snapshot.gpuVersion != null && _snapshot.gpuVersion!.isNotEmpty)
                _InfoLine(label: 'GL', value: _snapshot.gpuVersion!),
              if (vendorLabel != null)
                _InfoLine(label: 'Vendor', value: vendorLabel),
              if (deviceLabel != null)
                _InfoLine(label: 'Device', value: deviceLabel),
              if (_hasChannel)
                _InfoLine(
                  label: 'Pipeline',
                  value: _snapshot.eglReady == true ? 'EGL · active' : 'EGL · standby',
                )
              else
                const _InfoLine(label: 'Pipeline', value: 'Channel unavailable'),
              _InfoLine(
                label: 'FFI/Ffigen',
                value: ffiEnabled ? 'enabled' : 'disabled',
              ),
              const SizedBox(height: 8),
              Row(
                children: [
                  Expanded(child: _MetricTile(title: 'FPS', value: _snapshot.fpsLabel)),
                  const SizedBox(width: 8),
                  Expanded(child: _MetricTile(title: 'Frame', value: _snapshot.frameTimeLabel)),
                ],
              ),
              if (surfaceLabel != null) ...[
                const SizedBox(height: 8),
                _InfoLine(label: 'Viewport', value: surfaceLabel),
              ],
              if (_snapshot.frameCountLabel != null) ...[
                const SizedBox(height: 8),
                _InfoLine(label: 'Frames', value: _snapshot.frameCountLabel!),
              ],
              if (_snapshot.partCount != null || _snapshot.constraintCount != null) ...[
                const SizedBox(height: 8),
                if (_snapshot.partCount != null)
                  _InfoLine(label: 'Parts', value: _snapshot.partCount!.toString()),
                if (_snapshot.constraintCount != null)
                  _InfoLine(label: 'Constraints', value: _snapshot.constraintCount!.toString()),
              ],
            ],
          ),
        ),
      ),
    );
  }
}

class _InfoLine extends StatelessWidget {
  const _InfoLine({required this.label, required this.value});

  final String label;
  final String value;

  @override
  Widget build(BuildContext context) {
    final baseStyle = Theme.of(context).textTheme.bodySmall;
    final labelColor = baseStyle?.color?.withOpacity(0.8);
    final labelStyle = baseStyle?.copyWith(
      fontWeight: FontWeight.w600,
      color: labelColor,
    );
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 2),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('$label:', style: labelStyle),
          const SizedBox(width: 6),
          Expanded(
            child: Text(
              value,
              style: baseStyle,
              maxLines: 2,
              overflow: TextOverflow.ellipsis,
            ),
          ),
        ],
      ),
    );
  }
}

class _MetricTile extends StatelessWidget {
  const _MetricTile({required this.title, required this.value});

  final String title;
  final String value;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
      decoration: BoxDecoration(
        color: theme.colorScheme.primary.withOpacity(0.08),
        borderRadius: BorderRadius.circular(10),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(
            title,
            style: theme.textTheme.labelSmall?.copyWith(fontSize: 11, color: theme.colorScheme.primary.withOpacity(0.7)),
          ),
          const SizedBox(height: 4),
          Text(
            value,
            style: theme.textTheme.bodyMedium?.copyWith(fontWeight: FontWeight.w600),
          ),
        ],
      ),
    );
  }
}
