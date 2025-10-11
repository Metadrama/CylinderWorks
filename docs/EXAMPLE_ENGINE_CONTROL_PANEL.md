# Example: Building an Engine Control Panel

This document shows a practical example of building a complete engine control panel using the CylinderWorks design system.

## Goal

Create a professional control panel that allows users to:
- View real-time engine metrics (RPM, torque, temperature)
- Control throttle position with a slider
- Toggle engine on/off
- See engine status indicators

## Step-by-Step Implementation

### Step 1: Create the Control Panel Widget

```dart
// lib/ui/engine_control_panel.dart
import 'package:flutter/material.dart';
import '../theme/design_tokens.dart';
import '../theme/app_theme.dart';
import 'components/components.dart';

class EngineControlPanel extends StatefulWidget {
  const EngineControlPanel({super.key});

  @override
  State<EngineControlPanel> createState() => _EngineControlPanelState();
}

class _EngineControlPanelState extends State<EngineControlPanel> {
  bool _engineRunning = false;
  double _throttle = 0.0;
  
  // Simulated engine data (replace with real data from native)
  double get _rpm => _engineRunning ? 1000 + (_throttle * 4500) : 0;
  double get _torque => _engineRunning ? _throttle * 95.5 : 0;
  double get _temperature => _engineRunning ? 75 + (_throttle * 20) : 25;
  double get _pressure => _engineRunning ? 1.5 + (_throttle * 1.5) : 0;

  @override
  Widget build(BuildContext context) {
    return InfoPanel(
      title: 'Engine Control',
      icon: Icons.settings_input_component,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // Status row
          _buildStatusRow(),
          
          const SizedBox(height: DesignTokens.spaceMd),
          
          // Metrics grid
          _buildMetricsGrid(),
          
          const SizedBox(height: DesignTokens.spaceMd),
          
          // Throttle control
          _buildThrottleControl(),
          
          const SizedBox(height: DesignTokens.spaceMd),
          
          // Engine toggle
          _buildEngineToggle(),
        ],
      ),
    );
  }

  Widget _buildStatusRow() {
    return Row(
      children: [
        Container(
          width: DesignTokens.iconSm,
          height: DesignTokens.iconSm,
          decoration: BoxDecoration(
            color: _engineRunning ? AppColors.success : AppColors.error,
            shape: BoxShape.circle,
            boxShadow: _engineRunning
                ? [
                    BoxShadow(
                      color: AppColors.success.withOpacity(0.5),
                      blurRadius: 8,
                      spreadRadius: 2,
                    ),
                  ]
                : null,
          ),
        ),
        const SizedBox(width: DesignTokens.spaceSm),
        Text(
          _engineRunning ? 'Running' : 'Stopped',
          style: Theme.of(context).textTheme.labelLarge?.copyWith(
                color: _engineRunning ? AppColors.success : AppColors.onSurfaceVariant,
              ),
        ),
        const Spacer(),
        if (_engineRunning)
          Container(
            padding: const EdgeInsets.symmetric(
              horizontal: DesignTokens.spaceSm,
              vertical: DesignTokens.spaceXs,
            ),
            decoration: BoxDecoration(
              color: AppColors.success.withOpacity(0.15),
              borderRadius: BorderRadius.circular(DesignTokens.radiusSm),
              border: Border.all(
                color: AppColors.success.withOpacity(0.3),
                width: 1,
              ),
            ),
            child: Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                Icon(
                  Icons.check_circle,
                  size: DesignTokens.iconXs,
                  color: AppColors.success,
                ),
                const SizedBox(width: DesignTokens.spaceXs),
                Text(
                  'Active',
                  style: Theme.of(context).textTheme.labelSmall?.copyWith(
                        color: AppColors.success,
                        fontWeight: FontWeight.w600,
                      ),
                ),
              ],
            ),
          ),
      ],
    );
  }

  Widget _buildMetricsGrid() {
    return Column(
      children: [
        Row(
          children: [
            Expanded(
              child: MetricTile(
                title: 'RPM',
                value: _rpm.toStringAsFixed(0),
                unit: 'rpm',
                color: AppColors.dataRpm,
                icon: Icons.speed,
              ),
            ),
            const SizedBox(width: DesignTokens.spaceSm),
            Expanded(
              child: MetricTile(
                title: 'Torque',
                value: _torque.toStringAsFixed(1),
                unit: 'Nm',
                color: AppColors.dataTorque,
                icon: Icons.settings,
              ),
            ),
          ],
        ),
        const SizedBox(height: DesignTokens.spaceSm),
        Row(
          children: [
            Expanded(
              child: MetricTile(
                title: 'Temp',
                value: _temperature.toStringAsFixed(0),
                unit: '°C',
                color: _temperature > 90 ? AppColors.warning : AppColors.dataTemperature,
                icon: Icons.thermostat,
              ),
            ),
            const SizedBox(width: DesignTokens.spaceSm),
            Expanded(
              child: MetricTile(
                title: 'Pressure',
                value: _pressure.toStringAsFixed(1),
                unit: 'bar',
                color: AppColors.dataPressure,
                icon: Icons.water_drop,
              ),
            ),
          ],
        ),
      ],
    );
  }

  Widget _buildThrottleControl() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(
              'Throttle Position',
              style: Theme.of(context).textTheme.labelLarge,
            ),
            Container(
              padding: const EdgeInsets.symmetric(
                horizontal: DesignTokens.spaceSm,
                vertical: DesignTokens.spaceXs / 2,
              ),
              decoration: BoxDecoration(
                color: AppColors.primary.withOpacity(0.15),
                borderRadius: BorderRadius.circular(DesignTokens.radiusSm),
              ),
              child: Text(
                '${(_throttle * 100).toStringAsFixed(0)}%',
                style: AppTheme.monoMedium(context).copyWith(
                  color: AppColors.primary,
                  fontSize: 13,
                ),
              ),
            ),
          ],
        ),
        const SizedBox(height: DesignTokens.spaceXs),
        SliderTheme(
          data: SliderTheme.of(context).copyWith(
            trackHeight: 6,
            thumbShape: const RoundSliderThumbShape(
              enabledThumbRadius: 10,
            ),
            overlayShape: const RoundSliderOverlayShape(
              overlayRadius: 20,
            ),
          ),
          child: Slider(
            value: _throttle,
            onChanged: _engineRunning
                ? (value) => setState(() => _throttle = value)
                : null,
            min: 0,
            max: 1,
            divisions: 100,
            activeColor: AppColors.primary,
            inactiveColor: AppColors.surfaceVariant,
          ),
        ),
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(
              'Idle',
              style: Theme.of(context).textTheme.labelSmall,
            ),
            Text(
              'Full',
              style: Theme.of(context).textTheme.labelSmall,
            ),
          ],
        ),
      ],
    );
  }

  Widget _buildEngineToggle() {
    return SizedBox(
      width: double.infinity,
      child: FilledButton.icon(
        onPressed: () => setState(() => _engineRunning = !_engineRunning),
        icon: Icon(_engineRunning ? Icons.stop : Icons.play_arrow),
        label: Text(_engineRunning ? 'Stop Engine' : 'Start Engine'),
        style: FilledButton.styleFrom(
          backgroundColor: _engineRunning ? AppColors.error : AppColors.success,
          foregroundColor: Colors.white,
          padding: const EdgeInsets.symmetric(
            vertical: DesignTokens.spaceSm + DesignTokens.spaceXs,
          ),
        ),
      ),
    );
  }
}
```

### Step 2: Add to Main Screen

```dart
// lib/main.dart (add to EngineRendererScreen)

@override
Widget build(BuildContext context) {
  return Scaffold(
    body: Stack(
      children: [
        const Positioned.fill(
          child: EngineRendererView(),
        ),
        
        // ... existing overlays ...
        
        // Add engine control panel
        Positioned(
          left: DesignTokens.spaceMd,
          bottom: DesignTokens.spaceLg,
          child: EngineControlPanel(),
        ),
      ],
    ),
  );
}
```

## Result

The engine control panel will have:

### Visual Features
- ✅ Professional card layout with rounded corners
- ✅ Status indicator with glow effect when running
- ✅ Four real-time metrics in a 2x2 grid
- ✅ Color-coded metrics (blue, purple, red, green)
- ✅ Interactive throttle slider with percentage display
- ✅ Large, accessible start/stop button
- ✅ Consistent spacing using design tokens
- ✅ Smooth animations and transitions

### Layout Breakdown

```
┌─────────────────────────────────────┐
│ 🔧 Engine Control              [×]  │  ← InfoPanel header
├─────────────────────────────────────┤
│ ● Running          [Active]         │  ← Status row
│                                     │
│ ┌──────────┐  ┌──────────┐        │
│ │ 🏃 RPM    │  │ ⚙️ Torque│        │  ← Metrics row 1
│ │ 3450 rpm │  │ 71.9 Nm  │        │
│ └──────────┘  └──────────┘        │
│                                     │
│ ┌──────────┐  ┌──────────┐        │
│ │ 🌡️ Temp  │  │ 💧 Press │        │  ← Metrics row 2
│ │ 87 °C    │  │ 2.3 bar  │        │
│ └──────────┘  └──────────┘        │
│                                     │
│ Throttle Position          [65%]   │  ← Slider label
│ ━━━━━━━━●━━━━━━━━━━━━━          │  ← Slider
│ Idle                    Full       │
│                                     │
│ [  ⏹  Stop Engine  ]               │  ← Action button
└─────────────────────────────────────┘
```

## Responsive Behavior

### Mobile (< 600px)
- Panel width: 280-320px (constrained by InfoPanel)
- Metrics: 2x2 grid with small tiles
- Single column layout

### Tablet (600-900px)
- Panel width: 320px
- Metrics: 2x2 grid with medium tiles
- Can show side-by-side with other panels

### Desktop (> 900px)
- Panel width: 320px
- Metrics: 2x2 grid with larger tiles
- Multiple panels in a grid layout

## Advanced Features

### Add Real-time Updates

```dart
class _EngineControlPanelState extends State<EngineControlPanel> {
  Timer? _updateTimer;
  
  @override
  void initState() {
    super.initState();
    // Poll engine data every 100ms
    _updateTimer = Timer.periodic(
      const Duration(milliseconds: 100),
      (_) => _updateEngineData(),
    );
  }
  
  @override
  void dispose() {
    _updateTimer?.cancel();
    super.dispose();
  }
  
  void _updateEngineData() {
    if (!_engineRunning) return;
    
    // Get real data from native engine
    final data = EngineBindings.instance.getEngineState();
    setState(() {
      _rpm = data.rpm;
      _torque = data.torque;
      _temperature = data.temperature;
      _pressure = data.pressure;
    });
  }
}
```

### Add Warning States

```dart
Widget _buildMetricsGrid() {
  // Check for warning conditions
  final bool tempWarning = _temperature > 90;
  final bool pressureWarning = _pressure < 1.0 || _pressure > 3.0;
  
  return Column(
    children: [
      // Show warning banner if needed
      if (tempWarning || pressureWarning) ...[
        Container(
          padding: const EdgeInsets.all(DesignTokens.spaceSm),
          decoration: BoxDecoration(
            color: AppColors.warning.withOpacity(0.15),
            borderRadius: BorderRadius.circular(DesignTokens.radiusSm),
            border: Border.all(
              color: AppColors.warning.withOpacity(0.3),
              width: 1,
            ),
          ),
          child: Row(
            children: [
              Icon(Icons.warning, size: DesignTokens.iconSm, color: AppColors.warning),
              const SizedBox(width: DesignTokens.spaceSm),
              Expanded(
                child: Text(
                  tempWarning ? 'Temperature high!' : 'Pressure abnormal!',
                  style: Theme.of(context).textTheme.labelMedium?.copyWith(
                    color: AppColors.warning,
                    fontWeight: FontWeight.w600,
                  ),
                ),
              ),
            ],
          ),
        ),
        const SizedBox(height: DesignTokens.spaceSm),
      ],
      
      // Metrics grid...
    ],
  );
}
```

### Add Animation

```dart
class _EngineControlPanelState extends State<EngineControlPanel>
    with SingleTickerProviderStateMixin {
  
  late AnimationController _pulseController;
  
  @override
  void initState() {
    super.initState();
    _pulseController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 1000),
    )..repeat(reverse: true);
  }
  
  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _pulseController,
      builder: (context, child) {
        return InfoPanel(
          // ... panel content
          child: Column(
            children: [
              // Status indicator with pulse animation
              Container(
                width: DesignTokens.iconSm,
                height: DesignTokens.iconSm,
                decoration: BoxDecoration(
                  color: _engineRunning ? AppColors.success : AppColors.error,
                  shape: BoxShape.circle,
                  boxShadow: _engineRunning
                      ? [
                          BoxShadow(
                            color: AppColors.success.withOpacity(
                              0.3 + (_pulseController.value * 0.3),
                            ),
                            blurRadius: 8 + (_pulseController.value * 4),
                            spreadRadius: 2 + (_pulseController.value * 2),
                          ),
                        ]
                      : null,
                ),
              ),
              // ... rest of content
            ],
          ),
        );
      },
    );
  }
}
```

## Integration with Native Engine

```dart
// Example of connecting to native engine via FFI
class EngineState {
  final double rpm;
  final double torque;
  final double temperature;
  final double pressure;
  final bool running;
  
  EngineState({
    required this.rpm,
    required this.torque,
    required this.temperature,
    required this.pressure,
    required this.running,
  });
}

class EngineBindings {
  static final instance = EngineBindings._();
  EngineBindings._();
  
  // FFI function to get engine state
  EngineState getEngineState() {
    // Call native function via FFI
    // This is a placeholder - implement actual FFI binding
    return EngineState(
      rpm: 0.0,
      torque: 0.0,
      temperature: 25.0,
      pressure: 0.0,
      running: false,
    );
  }
  
  // FFI function to set throttle
  void setThrottle(double position) {
    // Call native function via FFI
  }
  
  // FFI function to start/stop engine
  void setEngineRunning(bool running) {
    // Call native function via FFI
  }
}
```

## Summary

This example demonstrates:

1. ✅ Using `InfoPanel` for professional container
2. ✅ Using `MetricTile` for real-time data display
3. ✅ Consistent spacing with `DesignTokens`
4. ✅ Semantic colors from `AppColors`
5. ✅ Theme-aware text styles
6. ✅ Monospace fonts for numeric values
7. ✅ Interactive controls (slider, button)
8. ✅ Status indicators with visual feedback
9. ✅ Warning states and animations
10. ✅ Responsive design considerations

**Result:** A professional, production-ready engine control panel that respects the design system and provides excellent UX for both enthusiasts and professionals.

---

**Example Version:** 1.0  
**Last Updated:** 2025-10-11
