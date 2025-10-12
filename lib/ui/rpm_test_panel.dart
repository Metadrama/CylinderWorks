import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

/// Lightweight harness to send RPM commands to the native test kinematics module.
class RpmTestPanel extends StatefulWidget {
  const RpmTestPanel({super.key});

  @override
  State<RpmTestPanel> createState() => _RpmTestPanelState();
}

class _RpmTestPanelState extends State<RpmTestPanel> {
  static const MethodChannel _channel = MethodChannel('engine/control');
  static const double _maxRpm = 12000;

  double _rpm = 0;
  Timer? _debounce;
  bool _channelAvailable = true;

  @override
  void dispose() {
    _debounce?.cancel();
    _sendRpm(0);
    super.dispose();
  }

  void _onChanged(double value) {
    setState(() {
      _rpm = value;
    });
    _debounce?.cancel();
    _debounce = Timer(const Duration(milliseconds: 80), () => _sendRpm(_rpm));
  }

  Future<void> _sendRpm(double value) async {
    try {
      await _channel.invokeMethod<void>('setTestRpm', {'rpm': value});
      if (!_channelAvailable) {
        setState(() => _channelAvailable = true);
      }
    } on MissingPluginException {
      if (_channelAvailable) {
        setState(() => _channelAvailable = false);
      }
    } catch (_) {
      // Ignore errors for debug tooling.
    }
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final int divisions = (_maxRpm / 100).round();
    return Card(
      color: theme.colorScheme.surface.withOpacity(0.9),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(14)),
      elevation: 6,
      child: SizedBox(
        width: 260,
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                children: [
                  const Icon(Icons.auto_fix_high_outlined, size: 18),
                  const SizedBox(width: 8),
                  const Expanded(
                    child: Text(
                      'RPM Harness',
                      style: TextStyle(fontWeight: FontWeight.w600),
                    ),
                  ),
                  Text(
                    '${_rpm.round()} rpm',
                    style: theme.textTheme.labelSmall,
                  ),
                ],
              ),
              const SizedBox(height: 12),
              Slider(
                value: _rpm.clamp(0, _maxRpm),
                min: 0,
                max: _maxRpm,
                divisions: divisions > 0 ? divisions : null,
                label: '${_rpm.round()} rpm',
                onChanged: _onChanged,
              ),
              const SizedBox(height: 4),
              if (!_channelAvailable)
                Text(
                  'Control channel unavailable on this build.',
                  style: theme.textTheme.bodySmall?.copyWith(
                    color: theme.colorScheme.error,
                  ),
                )
              else
                Text(
                  'Slide to spin the test kinematics. Stops when returning to 0 rpm.',
                  style: theme.textTheme.bodySmall?.copyWith(
                    color: theme.colorScheme.onSurface.withOpacity(0.7),
                  ),
                ),
            ],
          ),
        ),
      ),
    );
  }
}
