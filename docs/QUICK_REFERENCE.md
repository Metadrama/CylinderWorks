# CylinderWorks Design System - Quick Reference

## Import Statements

```dart
// Theme system
import 'package:cylinderworks/theme/app_theme.dart';
import 'package:cylinderworks/theme/design_tokens.dart';
import 'package:cylinderworks/theme/theme_extensions.dart';

// Components (all at once)
import 'package:cylinderworks/ui/components/components.dart';

// Individual components
import 'package:cylinderworks/ui/components/metric_tile.dart';
import 'package:cylinderworks/ui/components/info_panel.dart';
import 'package:cylinderworks/ui/components/info_line.dart';
```

## Spacing (8-point grid)

| Token | Value | Use Case |
|-------|-------|----------|
| `DesignTokens.spaceXs` | 4 | Tight padding, small gaps |
| `DesignTokens.spaceSm` | 8 | Standard padding inside components |
| `DesignTokens.spaceMd` | 16 | Component margins, content spacing |
| `DesignTokens.spaceLg` | 24 | Section spacing |
| `DesignTokens.spaceXl` | 32 | Major section spacing |
| `DesignTokens.spaceXxl` | 48 | Screen-level spacing |

```dart
// Examples
Padding(padding: EdgeInsets.all(DesignTokens.spaceMd))
SizedBox(height: DesignTokens.spaceSm)
EdgeInsets.symmetric(horizontal: DesignTokens.spaceMd, vertical: DesignTokens.spaceSm)
```

## Border Radius

| Token | Value | Use Case |
|-------|-------|----------|
| `DesignTokens.radiusSm` | 8 | Buttons, chips, small cards |
| `DesignTokens.radiusMd` | 12 | Standard cards, panels |
| `DesignTokens.radiusLg` | 16 | Large containers, dialogs |
| `DesignTokens.radiusXl` | 20 | Hero elements |
| `DesignTokens.radiusFull` | 9999 | Pills, circular buttons |

```dart
// Examples
BorderRadius.circular(DesignTokens.radiusMd)
RoundedRectangleBorder(borderRadius: BorderRadius.circular(DesignTokens.radiusSm))
```

## Colors

### Primary (Technical Blue)
```dart
AppColors.primary        // #2563EB - Main brand color
AppColors.primaryLight   // #60A5FA - Lighter variant
AppColors.primaryDark    // #1E40AF - Darker variant
AppColors.onPrimary      // #FFFFFF - Text on primary
```

### Surface (Dark Mode)
```dart
AppColors.background         // #08090D - App background
AppColors.surface            // #1A1C1E - Main surface
AppColors.surfaceVariant     // #2D3135 - Elevated surfaces
AppColors.surfaceBright      // #3A3E42 - Brightest surface
AppColors.onSurface          // #E4E6E8 - Text on surface
AppColors.onSurfaceVariant   // #B8BABD - Muted text
```

### Semantic
```dart
AppColors.error      // #EF4444 - Errors, critical states
AppColors.success    // #10B981 - Success, active states
AppColors.warning    // #F59E0B - Warnings, attention
AppColors.info       // #3B82F6 - Info, neutral emphasis
```

### Data Visualization
```dart
AppColors.dataRpm            // #3B82F6 - Blue (RPM)
AppColors.dataTorque         // #8B5CF6 - Purple (Torque)
AppColors.dataTemperature    // #EF4444 - Red (Temperature)
AppColors.dataPressure       // #10B981 - Green (Pressure)
AppColors.dataEfficiency     // #F59E0B - Amber (Efficiency)
AppColors.dataFuel           // #06B6D4 - Cyan (Fuel)
```

## Typography

### Display (Hero metrics, large numbers)
```dart
Theme.of(context).textTheme.displayLarge   // 48sp, Weight: 700
Theme.of(context).textTheme.displayMedium  // 36sp, Weight: 600
Theme.of(context).textTheme.displaySmall   // 24sp, Weight: 600
```

### Headings
```dart
Theme.of(context).textTheme.headlineLarge   // 20sp, Weight: 600
Theme.of(context).textTheme.headlineMedium  // 18sp, Weight: 600
Theme.of(context).textTheme.headlineSmall   // 16sp, Weight: 600
```

### Body
```dart
Theme.of(context).textTheme.bodyLarge   // 16sp, Weight: 400
Theme.of(context).textTheme.bodyMedium  // 14sp, Weight: 400
Theme.of(context).textTheme.bodySmall   // 12sp, Weight: 400
```

### Labels
```dart
Theme.of(context).textTheme.labelLarge   // 14sp, Weight: 500
Theme.of(context).textTheme.labelMedium  // 12sp, Weight: 500
Theme.of(context).textTheme.labelSmall   // 11sp, Weight: 500
```

### Monospace (Numeric data)
```dart
AppTheme.monoLarge(context)   // 16sp, Roboto Mono
AppTheme.monoMedium(context)  // 14sp, Roboto Mono
AppTheme.monoSmall(context)   // 12sp, Roboto Mono
```

## Elevation

```dart
DesignTokens.elevationNone     // 0  - Flat
DesignTokens.elevationLow      // 2  - Subtle lift
DesignTokens.elevationMedium   // 4  - Standard cards
DesignTokens.elevationHigh     // 8  - Overlays, dialogs
DesignTokens.elevationHighest  // 16 - FABs, important elements
```

## Icons

```dart
DesignTokens.iconXs   // 16 - Extra small
DesignTokens.iconSm   // 18 - Small
DesignTokens.iconMd   // 24 - Medium (default)
DesignTokens.iconLg   // 32 - Large
DesignTokens.iconXl   // 48 - Extra large
```

## Opacity

```dart
DesignTokens.opacityDisabled  // 0.38 - Disabled state
DesignTokens.opacityMuted     // 0.6  - Muted text/icons
DesignTokens.opacitySubtle    // 0.8  - Subtle emphasis
DesignTokens.opacityOverlay   // 0.9  - Panel backgrounds
```

## Breakpoints

```dart
DesignTokens.breakpointMobile   // 0   - Mobile (0-600px)
DesignTokens.breakpointTablet   // 600 - Tablet (600-900px)
DesignTokens.breakpointDesktop  // 900 - Desktop (900px+)

// Usage
if (MediaQuery.of(context).size.width < DesignTokens.breakpointTablet) {
  // Mobile layout
} else {
  // Tablet/Desktop layout
}
```

## Components

### MetricTile
```dart
MetricTile(
  title: 'Engine Speed',        // Required: Label
  value: '3450',                 // Required: Numeric value
  unit: 'RPM',                   // Optional: Unit
  icon: Icons.speed,             // Optional: Icon
  color: AppColors.dataRpm,      // Optional: Accent color
  isLoading: false,              // Optional: Loading state
  onTap: () => showDetails(),    // Optional: Tap handler
)
```

### InfoPanel
```dart
InfoPanel(
  title: 'Diagnostics',         // Optional: Panel title
  icon: Icons.monitor_heart,     // Optional: Title icon
  onClose: () => close(),        // Optional: Close button
  child: Column(                 // Required: Content
    children: [
      InfoLine(label: 'Status', value: 'OK'),
    ],
  ),
)
```

### InfoLine
```dart
InfoLine(
  label: 'Rendering',            // Required: Label
  value: 'OpenGL ES 3.0',        // Required: Value
  icon: Icons.info,              // Optional: Leading icon
  valueColor: AppColors.success, // Optional: Value color
)
```

## Common Patterns

### Card with Padding
```dart
Card(
  child: Padding(
    padding: const EdgeInsets.all(DesignTokens.spaceMd),
    child: content,
  ),
)
```

### Spaced Column
```dart
Column(
  children: [
    widget1,
    const SizedBox(height: DesignTokens.spaceMd),
    widget2,
    const SizedBox(height: DesignTokens.spaceMd),
    widget3,
  ],
)
```

### Metric Grid (2x2)
```dart
Column(
  children: [
    Row(
      children: [
        Expanded(child: MetricTile(...)),
        const SizedBox(width: DesignTokens.spaceSm),
        Expanded(child: MetricTile(...)),
      ],
    ),
    const SizedBox(height: DesignTokens.spaceSm),
    Row(
      children: [
        Expanded(child: MetricTile(...)),
        const SizedBox(width: DesignTokens.spaceSm),
        Expanded(child: MetricTile(...)),
      ],
    ),
  ],
)
```

### Status Indicator
```dart
Row(
  children: [
    Container(
      width: DesignTokens.iconSm,
      height: DesignTokens.iconSm,
      decoration: BoxDecoration(
        color: isActive ? AppColors.success : AppColors.error,
        shape: BoxShape.circle,
      ),
    ),
    const SizedBox(width: DesignTokens.spaceSm),
    Text(isActive ? 'Active' : 'Inactive'),
  ],
)
```

### Warning Banner
```dart
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
      Icon(Icons.warning, color: AppColors.warning),
      const SizedBox(width: DesignTokens.spaceSm),
      Expanded(child: Text('Warning message')),
    ],
  ),
)
```

## Animation Durations

```dart
DesignTokens.durationFast    // 150ms - Quick transitions
DesignTokens.durationNormal  // 250ms - Standard transitions
DesignTokens.durationSlow    // 350ms - Slow, emphasized transitions

// Usage
AnimatedContainer(
  duration: Duration(milliseconds: DesignTokens.durationNormal),
  curve: Curves.easeInOut,
  // ...
)
```

## Responsive Helpers

```dart
// Get screen width
final width = MediaQuery.of(context).size.width;

// Check if mobile
final isMobile = width < DesignTokens.breakpointTablet;

// Check if tablet
final isTablet = width >= DesignTokens.breakpointTablet &&
                 width < DesignTokens.breakpointDesktop;

// Check if desktop
final isDesktop = width >= DesignTokens.breakpointDesktop;

// Responsive spacing
final spacing = isMobile ? DesignTokens.spaceMd : DesignTokens.spaceLg;
```

## Accessibility

### Minimum Touch Target
```dart
const minTouchTarget = DesignTokens.minTouchTarget;  // 48x48

// Ensure interactive elements meet minimum
SizedBox(
  width: minTouchTarget,
  height: minTouchTarget,
  child: IconButton(...),
)
```

### Semantic Labels
```dart
IconButton(
  icon: Icon(Icons.close),
  tooltip: 'Close panel',  // Screen reader support
  onPressed: onClose,
)

Semantics(
  label: 'Engine speed: 3450 RPM',
  child: MetricTile(...),
)
```

## Theme Access

### Get Theme
```dart
final theme = Theme.of(context);
final colorScheme = theme.colorScheme;
final textTheme = theme.textTheme;
```

### Get Engineering Theme
```dart
final engineeringTheme = context.engineeringTheme;
final rpmColor = engineeringTheme.dataRpmColor;
```

## Don'ts ❌

```dart
// Don't use hardcoded values
padding: EdgeInsets.all(16.0)  // ❌
color: Color(0xFF2563EB)       // ❌
fontSize: 14                   // ❌

// Don't use inline styles
TextStyle(fontSize: 16, fontWeight: FontWeight.bold)  // ❌

// Don't ignore const
SizedBox(height: 8)  // ❌
```

## Do's ✅

```dart
// Use design tokens
padding: const EdgeInsets.all(DesignTokens.spaceMd)  // ✅
color: AppColors.primary                              // ✅

// Use theme typography
Theme.of(context).textTheme.bodyMedium  // ✅

// Use const constructors
const SizedBox(height: DesignTokens.spaceSm)  // ✅
```

## Quick Copy-Paste Templates

### Simple Metric Display
```dart
MetricTile(
  title: 'Label',
  value: '1234',
  unit: 'unit',
  color: AppColors.primary,
)
```

### Panel with Metrics
```dart
InfoPanel(
  title: 'Title',
  icon: Icons.info,
  child: Column(
    children: [
      InfoLine(label: 'Label', value: 'Value'),
      const SizedBox(height: DesignTokens.spaceSm),
      Row(
        children: [
          Expanded(child: MetricTile(title: 'M1', value: '123')),
          const SizedBox(width: DesignTokens.spaceSm),
          Expanded(child: MetricTile(title: 'M2', value: '456')),
        ],
      ),
    ],
  ),
)
```

### Action Button
```dart
SizedBox(
  width: double.infinity,
  child: FilledButton.icon(
    onPressed: onPressed,
    icon: Icon(Icons.play_arrow),
    label: Text('Start'),
  ),
)
```

---

**Version:** 1.0  
**Print this for quick reference while coding!**
