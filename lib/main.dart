import 'package:flutter/material.dart';

import 'native/engine_renderer_bindings.dart';
import 'native/engine_renderer_view.dart';
import 'ui/monitoring_overlay.dart';
import 'ui/rpm_test_panel.dart';

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
      theme: ThemeData.dark().copyWith(
        scaffoldBackgroundColor: const Color(0xFF08090D),
      ),
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
          const Positioned.fill(child: EngineRendererView()),
          Positioned(
            left: 16,
            top: 48,
            child: SizedBox(
              width: 280,
              child: DecoratedBox(
                decoration: BoxDecoration(
                  color: Colors.black.withOpacity(0.35),
                  borderRadius: BorderRadius.circular(12),
                ),
                child: const Padding(
                  padding: EdgeInsets.all(12),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      Text(
                        'CylinderWorks Renderer Prototype',
                        style: TextStyle(
                          fontSize: 16,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      SizedBox(height: 8),
                      Text('• One finger drag: orbit camera'),
                      Text('• Two finger drag: pan target'),
                      Text('• Pinch: zoom'),
                    ],
                  ),
                ),
              ),
            ),
          ),
          if (_showDiagnostics)
            Positioned(
              top: 48,
              right: 16,
              child: MonitoringOverlay(
                onClose: () => setState(() => _showDiagnostics = false),
              ),
            ),
          if (!_showDiagnostics)
            Positioned(
              bottom: 32,
              right: 24,
              child: FloatingActionButton.small(
                heroTag: 'monitoring-toggle',
                backgroundColor: Colors.black.withOpacity(0.65),
                onPressed: () => setState(() => _showDiagnostics = true),
                child: const Icon(Icons.monitor_heart_outlined),
              ),
            ),
          const Positioned(left: 16, bottom: 28, child: RpmTestPanel()),
        ],
      ),
    );
  }
}
