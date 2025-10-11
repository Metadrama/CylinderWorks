import 'package:flutter/material.dart';

import 'native/engine_renderer_bindings.dart';
import 'native/engine_renderer_view.dart';
import 'theme/app_theme.dart';
import 'theme/design_tokens.dart';
import 'ui/monitoring_overlay.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  EngineRendererBindings.instance.loadIfNeeded();
  runApp(const MainApp());
}

class MainApp extends StatelessWidget {
  const MainApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: AppTheme.darkTheme,
      home: const EngineRendererScreen(),
    );
  }
}

class EngineRendererScreen extends StatefulWidget {
  const EngineRendererScreen({super.key});

  @override
  State<EngineRendererScreen> createState() => _EngineRendererScreenState();
}

class _EngineRendererScreenState extends State<EngineRendererScreen> {
  bool _showDiagnostics = true;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Stack(
        children: [
          const Positioned.fill(
            child: EngineRendererView(),
          ),
          Positioned(
            left: DesignTokens.spaceMd,
            top: DesignTokens.spaceXxl,
            child: SizedBox(
              width: 280,
              child: DecoratedBox(
                decoration: BoxDecoration(
                  color: Colors.black.withOpacity(0.35),
                  borderRadius: BorderRadius.circular(DesignTokens.radiusMd),
                ),
                child: Padding(
                  padding: const EdgeInsets.all(DesignTokens.spaceSm + DesignTokens.spaceXs),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      Text(
                        'CylinderWorks Renderer Prototype',
                        style: Theme.of(context).textTheme.titleMedium?.copyWith(
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(height: DesignTokens.spaceSm),
                      const Text('• One finger drag: orbit camera'),
                      const Text('• Two finger drag: pan target'),
                      const Text('• Pinch: zoom'),
                    ],
                  ),
                ),
              ),
            ),
          ),
          if (_showDiagnostics)
            Positioned(
              top: DesignTokens.spaceXxl,
              right: DesignTokens.spaceMd,
              child: MonitoringOverlay(
                onClose: () => setState(() => _showDiagnostics = false),
              ),
            ),
          if (!_showDiagnostics)
            Positioned(
              bottom: DesignTokens.spaceXl,
              right: DesignTokens.spaceLg,
              child: FloatingActionButton.small(
                heroTag: 'monitoring-toggle',
                backgroundColor: Colors.black.withOpacity(0.65),
                onPressed: () => setState(() => _showDiagnostics = true),
                child: const Icon(Icons.monitor_heart_outlined),
              ),
            ),
        ],
      ),
    );
  }
}
