# CylinderWorks UI Implementation Guide

## Overview

This guide provides step-by-step instructions for implementing and using the CylinderWorks design system. The design system is built on Material 3 with custom extensions for mechanical engineering applications.

## Architecture

### Directory Structure

```
lib/
├── theme/
│   ├── app_theme.dart           # Main theme configuration with Material 3
│   ├── design_tokens.dart       # Spacing, colors, sizes constants
│   └── theme_extensions.dart    # Custom theme data for engineering UI
├── ui/
│   ├── components/              # Reusable UI components
│   │   ├── metric_tile.dart     # Numeric data display
│   │   ├── info_panel.dart      # Container for grouped info
│   │   └── info_line.dart       # Label-value pairs
│   └── monitoring_overlay.dart  # Example usage of design system
└── main.dart                    # App entry point
```

## Core Concepts

### 1. Design Tokens (Constants)

Design tokens are defined in `lib/theme/design_tokens.dart` and provide consistent values across the app.

#### Spacing (8-point grid)
```dart
DesignTokens.spaceXs   // 4.0  - Tight internal padding
DesignTokens.spaceSm   // 8.0  - Standard internal padding
DesignTokens.spaceMd   // 16.0 - Component spacing
DesignTokens.spaceLg   // 24.0 - Section spacing
DesignTokens.spaceXl   // 32.0 - Major section spacing
DesignTokens.spaceXxl  // 48.0 - Screen-level spacing
```

**Usage:**
```dart
Padding(
  padding: const EdgeInsets.all(DesignTokens.spaceMd),
  child: child,
)
```

#### Border Radius
```dart
DesignTokens.radiusSm   // 8.0  - Small components
DesignTokens.radiusMd   // 12.0 - Standard cards
DesignTokens.radiusLg   // 16.0 - Large containers
DesignTokens.radiusXl   // 20.0 - Hero elements
DesignTokens.radiusFull // 9999.0 - Pills/circular
```

**Usage:**
```dart
BorderRadius.circular(DesignTokens.radiusMd)
```

#### Elevation
```dart
DesignTokens.elevationLow      // 2.0  - Subtle lift
DesignTokens.elevationMedium   // 4.0  - Standard elevation
DesignTokens.elevationHigh     // 8.0  - Overlays
DesignTokens.elevationHighest  // 16.0 - FABs
```

### 2. Color System

Colors are defined in `lib/theme/design_tokens.dart` under `AppColors`.

#### Primary Colors (Technical Blue)
```dart
AppColors.primary       // Main brand color
AppColors.primaryLight  // Lighter variant
AppColors.primaryDark   // Darker variant
AppColors.onPrimary     // Text on primary
```

#### Surface Colors (Dark Mode)
```dart
AppColors.surface          // Main surface
AppColors.surfaceVariant   // Elevated surfaces
AppColors.onSurface        // Text on surface
AppColors.background       // App background
```

#### Semantic Colors
```dart
AppColors.error    // Error states
AppColors.success  // Success states
AppColors.warning  // Warning states
AppColors.info     // Info states
```

#### Data Visualization Colors
```dart
AppColors.dataRpm         // Blue
AppColors.dataTorque      // Purple
AppColors.dataTemperature // Red
AppColors.dataPressure    // Green
AppColors.dataEfficiency  // Amber
AppColors.dataFuel        // Cyan
```

**Usage:**
```dart
Container(
  color: AppColors.primary,
  child: Text(
    'Hello',
    style: TextStyle(color: AppColors.onPrimary),
  ),
)
```

### 3. Typography

Typography is configured in `AppTheme.darkTheme` and follows Material 3 naming.

#### Text Styles
```dart
// In widgets:
Text(
  'Display Large',
  style: Theme.of(context).textTheme.displayLarge,
)

Text(
  'Headline Medium',
  style: Theme.of(context).textTheme.headlineMedium,
)

Text(
  'Body text',
  style: Theme.of(context).textTheme.bodyMedium,
)

Text(
  'Label',
  style: Theme.of(context).textTheme.labelSmall,
)
```

#### Monospace for Numeric Data
```dart
Text(
  '1234.56',
  style: AppTheme.monoMedium(context),
)
```

### 4. Theme Extensions

Custom engineering-specific values are accessible via theme extensions.

```dart
// Access engineering theme:
final engineeringTheme = context.engineeringTheme;

// Use data visualization colors:
Container(
  color: engineeringTheme.dataRpmColor,
)
```

## Component Usage

### MetricTile

Display numeric values with labels, units, and optional icons.

```dart
MetricTile(
  title: 'Engine Speed',
  value: '3450',
  unit: 'RPM',
  icon: Icons.speed,
  color: AppColors.dataRpm,
)
```

**Properties:**
- `title` (required): Label for the metric
- `value` (required): Numeric value as string
- `unit` (optional): Unit of measurement
- `icon` (optional): Leading icon
- `color` (optional): Accent color (defaults to primary)
- `isLoading` (optional): Show loading state
- `onTap` (optional): Tap callback

**Example with loading:**
```dart
MetricTile(
  title: 'Torque',
  value: '--',
  unit: 'Nm',
  isLoading: true,
)
```

### InfoPanel

Container for grouped information with optional header.

```dart
InfoPanel(
  title: 'Diagnostics',
  icon: Icons.monitor_heart,
  onClose: () => print('Close panel'),
  child: Column(
    children: [
      InfoLine(label: 'Status', value: 'Running'),
      InfoLine(label: 'Mode', value: 'Auto'),
    ],
  ),
)
```

**Properties:**
- `child` (required): Content widget
- `title` (optional): Panel title
- `icon` (optional): Title icon
- `onClose` (optional): Close button callback
- `padding` (optional): Custom padding
- `constraints` (optional): Size constraints
- `backgroundColor` (optional): Custom background
- `elevation` (optional): Custom elevation

### InfoLine

Label-value pairs for structured data display.

```dart
InfoLine(
  label: 'Engine Type',
  value: 'Single Cylinder 115cc',
  icon: Icons.settings,
)
```

**Properties:**
- `label` (required): Left-side label
- `value` (required): Right-side value
- `icon` (optional): Leading icon
- `valueColor` (optional): Custom value text color

## Best Practices

### 1. Use Design Tokens Consistently

❌ **Don't:**
```dart
Padding(
  padding: EdgeInsets.all(16.0),  // Hardcoded value
)
```

✅ **Do:**
```dart
Padding(
  padding: const EdgeInsets.all(DesignTokens.spaceMd),
)
```

### 2. Use Theme Colors

❌ **Don't:**
```dart
Container(
  color: Color(0xFF2563EB),  // Hardcoded color
)
```

✅ **Do:**
```dart
Container(
  color: AppColors.primary,
)
```

Or use theme:
```dart
Container(
  color: Theme.of(context).colorScheme.primary,
)
```

### 3. Use Semantic Colors

For state-based coloring, use semantic colors:

```dart
// Success state
Icon(Icons.check, color: AppColors.success)

// Warning state  
Icon(Icons.warning, color: AppColors.warning)

// Error state
Icon(Icons.error, color: AppColors.error)
```

### 4. Maintain Touch Targets

Ensure interactive elements meet minimum 48x48 logical pixel size:

```dart
IconButton(
  iconSize: 24,
  // Button automatically meets 48x48 minimum
  onPressed: () {},
  icon: Icon(Icons.settings),
)
```

### 5. Use const Constructors

For better performance, use `const` wherever possible:

```dart
const SizedBox(height: DesignTokens.spaceMd)
const Icon(Icons.speed)
```

## Responsive Design

### Breakpoints

Use design tokens for responsive layouts:

```dart
LayoutBuilder(
  builder: (context, constraints) {
    if (constraints.maxWidth < DesignTokens.breakpointTablet) {
      // Mobile layout
      return Column(children: tiles);
    } else {
      // Tablet/Desktop layout
      return GridView(children: tiles);
    }
  },
)
```

### Adaptive Spacing

Scale spacing based on screen size:

```dart
final spacing = MediaQuery.of(context).size.width < 600
    ? DesignTokens.spaceMd
    : DesignTokens.spaceLg;
```

## Adding New Components

### Step 1: Create Component File

Create a new file in `lib/ui/components/`:

```dart
// lib/ui/components/status_indicator.dart
import 'package:flutter/material.dart';
import '../../theme/design_tokens.dart';

class StatusIndicator extends StatelessWidget {
  const StatusIndicator({
    super.key,
    required this.status,
    this.size,
  });

  final String status;
  final double? size;

  @override
  Widget build(BuildContext context) {
    final indicatorSize = size ?? DesignTokens.iconSm;
    
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Container(
          width: indicatorSize,
          height: indicatorSize,
          decoration: BoxDecoration(
            color: _getStatusColor(),
            shape: BoxShape.circle,
          ),
        ),
        SizedBox(width: DesignTokens.spaceXs),
        Text(
          status,
          style: Theme.of(context).textTheme.labelMedium,
        ),
      ],
    );
  }

  Color _getStatusColor() {
    switch (status.toLowerCase()) {
      case 'running':
        return AppColors.success;
      case 'warning':
        return AppColors.warning;
      case 'error':
        return AppColors.error;
      default:
        return AppColors.info;
    }
  }
}
```

### Step 2: Export Component

Create or update `lib/ui/components/components.dart`:

```dart
export 'info_line.dart';
export 'info_panel.dart';
export 'metric_tile.dart';
export 'status_indicator.dart';
```

### Step 3: Use Component

```dart
import 'package:cylinderworks/ui/components/components.dart';

StatusIndicator(status: 'Running')
```

## Theming for Business Features

### Premium Content Indicator

```dart
Container(
  decoration: BoxDecoration(
    gradient: LinearGradient(
      colors: [
        AppColors.tertiary,
        AppColors.tertiaryLight,
      ],
    ),
    borderRadius: BorderRadius.circular(DesignTokens.radiusSm),
  ),
  padding: const EdgeInsets.symmetric(
    horizontal: DesignTokens.spaceSm,
    vertical: DesignTokens.spaceXs,
  ),
  child: Row(
    mainAxisSize: MainAxisSize.min,
    children: [
      Icon(Icons.star, size: DesignTokens.iconXs),
      SizedBox(width: DesignTokens.spaceXs),
      Text('Premium'),
    ],
  ),
)
```

### Ad Integration Zones

Reserve space for ads using consistent spacing:

```dart
Column(
  children: [
    Expanded(child: mainContent),
    
    // Ad zone
    SizedBox(height: DesignTokens.spaceLg),
    Container(
      height: 50,
      color: AppColors.surfaceVariant,
      alignment: Alignment.center,
      child: Text(
        'Ad Space',
        style: Theme.of(context).textTheme.labelSmall,
      ),
    ),
    SizedBox(height: DesignTokens.spaceMd),
    
    // Bottom controls
    bottomControls,
  ],
)
```

## Animation Guidelines

Use consistent animation durations:

```dart
AnimatedContainer(
  duration: Duration(milliseconds: DesignTokens.durationNormal),
  curve: Curves.easeInOut,
  color: isActive ? AppColors.primary : AppColors.surface,
)
```

## Accessibility

### Contrast Ratios

The theme ensures proper contrast ratios:
- Body text: 4.5:1 minimum ✓
- Large text: 3:1 minimum ✓
- Critical data: 7:1 on demand

### Semantic Labels

Add semantic labels for screen readers:

```dart
IconButton(
  icon: Icon(Icons.close),
  onPressed: onClose,
  tooltip: 'Close panel',  // Important for accessibility
)
```

### Text Scaling

Support system font size preferences:

```dart
Text(
  'Label',
  style: Theme.of(context).textTheme.bodyMedium,
  // Flutter automatically scales based on system settings
)
```

## Testing Your Implementation

### Visual Consistency Check

1. All spacing uses design tokens ✓
2. All colors use AppColors or theme.colorScheme ✓
3. All text uses theme.textTheme ✓
4. Interactive elements meet 48x48 minimum ✓
5. Border radius uses DesignTokens ✓

### Code Review Checklist

- [ ] No hardcoded spacing values
- [ ] No hardcoded colors (except very specific cases)
- [ ] No hardcoded font sizes
- [ ] Components use const where possible
- [ ] Responsive breakpoints considered
- [ ] Accessibility labels added
- [ ] Touch targets meet minimum size

## Migration from Old Code

### Before:
```dart
Container(
  padding: EdgeInsets.all(16),
  decoration: BoxDecoration(
    color: Color(0xFF2D3135),
    borderRadius: BorderRadius.circular(12),
  ),
  child: Text(
    'Hello',
    style: TextStyle(
      fontSize: 14,
      fontWeight: FontWeight.w600,
    ),
  ),
)
```

### After:
```dart
Container(
  padding: const EdgeInsets.all(DesignTokens.spaceMd),
  decoration: BoxDecoration(
    color: AppColors.surfaceVariant,
    borderRadius: BorderRadius.circular(DesignTokens.radiusMd),
  ),
  child: Text(
    'Hello',
    style: Theme.of(context).textTheme.labelLarge,
  ),
)
```

## Common Patterns

### Card with Metric Grid

```dart
InfoPanel(
  title: 'Engine Metrics',
  icon: Icons.engineering,
  child: Column(
    children: [
      Row(
        children: [
          Expanded(
            child: MetricTile(
              title: 'RPM',
              value: '3450',
              unit: 'rpm',
              color: AppColors.dataRpm,
            ),
          ),
          SizedBox(width: DesignTokens.spaceSm),
          Expanded(
            child: MetricTile(
              title: 'Torque',
              value: '85.2',
              unit: 'Nm',
              color: AppColors.dataTorque,
            ),
          ),
        ],
      ),
      SizedBox(height: DesignTokens.spaceSm),
      Row(
        children: [
          Expanded(
            child: MetricTile(
              title: 'Temp',
              value: '92',
              unit: '°C',
              color: AppColors.dataTemperature,
            ),
          ),
          SizedBox(width: DesignTokens.spaceSm),
          Expanded(
            child: MetricTile(
              title: 'Pressure',
              value: '2.5',
              unit: 'bar',
              color: AppColors.dataPressure,
            ),
          ),
        ],
      ),
    ],
  ),
)
```

### Overlay Positioning

```dart
Stack(
  children: [
    Positioned.fill(child: mainContent),
    
    // Top-left panel
    Positioned(
      left: DesignTokens.spaceMd,
      top: DesignTokens.spaceXxl,
      child: panel,
    ),
    
    // Top-right panel
    Positioned(
      right: DesignTokens.spaceMd,
      top: DesignTokens.spaceXxl,
      child: diagnostics,
    ),
    
    // Bottom FAB
    Positioned(
      right: DesignTokens.spaceLg,
      bottom: DesignTokens.spaceXl,
      child: FloatingActionButton(...),
    ),
  ],
)
```

## FAQ

**Q: Can I use custom colors for specific cases?**  
A: Yes, but prefer extending `EngineeringTheme` in `theme_extensions.dart` first. Only use hardcoded colors for one-off cases.

**Q: What if I need a spacing value not in design tokens?**  
A: Consider if your case truly needs a custom value, or if it can use a combination (e.g., `DesignTokens.spaceSm + DesignTokens.spaceXs`). If absolutely needed, add it to `DesignTokens`.

**Q: How do I handle very large screens (desktop)?**  
A: Use `LayoutBuilder` and `DesignTokens.breakpointDesktop` to create different layouts. Consider max content width: `DesignTokens.maxContentWidth`.

**Q: Should I use Material 3 components or create custom ones?**  
A: Use Material 3 components when they fit (Button, Card, etc.). Create custom components for domain-specific needs (MetricTile, etc.).

## Resources

- [Material Design 3 Guidelines](https://m3.material.io/)
- [Flutter Theme Documentation](https://docs.flutter.dev/cookbook/design/themes)
- [Accessibility Guidelines](https://docs.flutter.dev/development/accessibility-and-localization/accessibility)
- [8-Point Grid System](https://builttoadapt.io/intro-to-the-8-point-grid-system-d2573cde8632)

---

**Last Updated:** 2025-10-11  
**Version:** 1.0
