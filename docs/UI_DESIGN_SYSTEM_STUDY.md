# CylinderWorks UI/UX Design System Study

## Executive Summary

This document outlines a professional UI/UX design system for CylinderWorks, a Flutter-based interactive engine simulator targeting mobile platforms with sellable business models and ad integration.

## Current State Analysis

### Existing UI Components
- **Main Screen**: Stack-based layout with overlays
- **Monitoring Overlay**: Diagnostics panel with FPS, GPU info, device metrics
- **Controls Help**: Static instruction overlay
- **Styling**: Manual color values, inconsistent spacing, basic Material theming

### Current Issues
1. **Inconsistent spacing**: Hardcoded values (12, 16, 24, 32, 48)
2. **No design tokens**: Colors, typography, and spacing defined inline
3. **Limited component reusability**: Manual widget composition everywhere
4. **No grid system**: Absolute positioning without responsive guidelines
5. **Poor mobile optimization**: Fixed pixel values don't scale across devices
6. **Inconsistent visual hierarchy**: Mixed font sizes and weights

## UI Library Options for Flutter

### Option 1: Flutter's Material 3 Design System (Recommended Starting Point)

**Pros:**
- Built-in, zero dependencies
- Excellent theming support with `ThemeData`
- Responsive components out of the box
- Well-documented and maintained
- Supports custom design tokens via theme extensions
- Good mobile performance

**Cons:**
- Google-opinionated design language
- Requires customization for engineering-specific UX
- Not as "professional dashboard" focused

**Implementation Complexity:** Low
**Cost:** Free

### Option 2: Custom Design System with shadcn-inspired Flutter Components

**Approach:** Build custom component library inspired by shadcn/ui principles:
- Reusable, composable widgets
- Consistent design tokens
- Professional engineering aesthetic
- Data visualization focus

**Pros:**
- Complete control over look and feel
- Can tailor for mechanical engineering context
- Compact, professional appearance
- No external dependencies if built properly
- Can incorporate industry-standard data viz components

**Cons:**
- Requires initial development time
- Need to maintain custom components
- More code to write and test

**Implementation Complexity:** Medium
**Cost:** Development time

### Option 3: FluentUI Flutter (Microsoft Design)

**Pros:**
- Professional, data-focused design language
- Good for technical/engineering applications
- Clean, compact layouts
- Well-suited for desktop and mobile

**Cons:**
- Additional dependency (`fluent_ui` package)
- Less mobile-optimized than Material
- Smaller community than Material

**Implementation Complexity:** Medium
**Cost:** Free (MIT license)

### Option 4: GetWidget + Custom Theme

**Pros:**
- Pre-built components library
- Faster development
- Good mobile optimization
- Free and open source

**Cons:**
- Another dependency to manage
- May not fit mechanical engineering aesthetic
- Generic look without customization

**Implementation Complexity:** Low-Medium
**Cost:** Free

## Recommended Approach: Material 3 + Custom Theme Extensions

### Rationale
1. **Zero dependencies**: Uses Flutter's built-in Material 3
2. **Professional capability**: Theme extensions allow custom design tokens
3. **Mobile-first**: Material 3 is optimized for mobile
4. **Future-proof**: Can gradually add custom components
5. **Business model friendly**: Ad SDKs integrate well with Material apps

### Design System Specifications

#### Color Palette (Mechanical Engineering Theme)

```dart
// Primary: Technical blue-grey
primary: Color(0xFF2563EB),       // Blue for technical/precision
onPrimary: Color(0xFFFFFFFF),

// Surface: Dark mode optimized
surface: Color(0xFF1A1C1E),        // Deep background
surfaceVariant: Color(0xFF2D3135), // Elevated surfaces
onSurface: Color(0xFFE4E6E8),

// Accent colors
secondary: Color(0xFF10B981),      // Green for success/running state
tertiary: Color(0xFFF59E0B),       // Amber for warnings/attention
error: Color(0xFFEF4444),          // Red for errors/critical

// Data visualization
rpm: Color(0xFF3B82F6),            // Blue
torque: Color(0xFF8B5CF6),         // Purple  
temperature: Color(0xFFEF4444),    // Red
pressure: Color(0xFF10B981),       // Green
```

#### Typography Scale

```dart
// Display (Hero metrics)
displayLarge: 48sp, Weight: 700
displayMedium: 36sp, Weight: 600
displaySmall: 24sp, Weight: 600

// Headings
headlineLarge: 20sp, Weight: 600
headlineMedium: 18sp, Weight: 600
headlineSmall: 16sp, Weight: 600

// Body
bodyLarge: 16sp, Weight: 400
bodyMedium: 14sp, Weight: 400
bodySmall: 12sp, Weight: 400

// Labels (Compact data labels)
labelLarge: 14sp, Weight: 500
labelMedium: 12sp, Weight: 500
labelSmall: 11sp, Weight: 500

// Monospace (Numeric data)
monoLarge: 16sp, Weight: 500, Family: 'Roboto Mono'
monoMedium: 14sp, Weight: 500, Family: 'Roboto Mono'
```

#### Spacing System (8-point grid)

```dart
const spacing = {
  'xs': 4.0,    // Tight internal padding
  'sm': 8.0,    // Standard internal padding
  'md': 16.0,   // Component spacing
  'lg': 24.0,   // Section spacing
  'xl': 32.0,   // Major section spacing
  'xxl': 48.0,  // Screen-level spacing
};
```

#### Border Radius System

```dart
const radius = {
  'sm': 8.0,    // Small components (buttons, chips)
  'md': 12.0,   // Standard cards, panels
  'lg': 16.0,   // Large containers
  'xl': 20.0,   // Hero elements
  'full': 9999.0, // Pills/circular
};
```

#### Elevation System

```dart
const elevation = {
  'none': 0.0,
  'low': 2.0,     // Subtle lift (data tiles)
  'medium': 4.0,  // Standard elevation (cards)
  'high': 8.0,    // Overlays, modals
  'highest': 16.0, // FABs, important CTAs
};
```

## Component Design Patterns for Mechanical Engineering

### 1. Data Metric Tile
**Purpose:** Display single numeric value with label and optional unit

**Anatomy:**
- Compact container (100-120px width)
- Label (labelSmall, muted color)
- Value (monoMedium-monoLarge, prominent color)
- Optional unit (labelSmall, muted)
- Optional status indicator (color-coded dot)

**Use Cases:** RPM, Torque, Temperature, Pressure readings

### 2. Monitoring Panel
**Purpose:** Group related telemetry data

**Anatomy:**
- Card container with medium elevation
- Header with icon and title
- Close/minimize button
- Grid of data metric tiles (2-column on mobile)
- Optional graph/chart area

**Use Cases:** Real-time diagnostics, performance metrics

### 3. Control Panel
**Purpose:** User inputs and actions

**Anatomy:**
- Card container
- Section headers
- Sliders with value readouts
- Toggle switches with labels
- Action buttons

**Use Cases:** Engine configuration, simulation controls

### 4. Status Bar
**Purpose:** Persistent state indicators

**Anatomy:**
- Compact horizontal strip
- Small icons with optional badges
- Minimal text
- Color-coded states

**Use Cases:** Connection status, simulation state, warnings

## Layout Patterns

### Mobile-First Grid System

**Breakpoints:**
```dart
const breakpoints = {
  'mobile': 0,      // 0-600px
  'tablet': 600,    // 600-900px
  'desktop': 900,   // 900px+
};
```

**Grid Configuration:**
- Mobile: Single column with full-width components
- Tablet: 2-column grid for data tiles
- Desktop: 3-4 column grid with sidebar

### Z-Index Layers
```dart
const zIndex = {
  'background': 0,    // Renderer view
  'content': 1,       // Main UI overlays
  'controls': 10,     // Interactive panels
  'modal': 100,       // Dialogs, dropdowns
  'notification': 1000, // Toasts, alerts
};
```

## Accessibility Considerations

1. **Touch Targets:** Minimum 48x48 logical pixels
2. **Contrast Ratios:** 
   - Body text: 4.5:1 minimum
   - Large text: 3:1 minimum
   - Critical data: 7:1 for AA compliance
3. **Font Scaling:** Support system font size preferences
4. **Color Independence:** Don't rely solely on color for critical info

## Performance Guidelines

1. **Widget Rebuilds:** Use `const` constructors wherever possible
2. **Expensive Rendering:** Wrap complex metrics in `RepaintBoundary`
3. **Animation:** Use `AnimatedContainer` for smooth transitions
4. **Lists:** Use `ListView.builder` for scrollable data
5. **State Management:** Keep diagnostics polling efficient (1-2 Hz max)

## Implementation Roadmap

### Phase 1: Foundation (Minimal Changes)
- [ ] Create `lib/theme/` directory structure
- [ ] Define `app_theme.dart` with Material 3 theme
- [ ] Create `design_tokens.dart` with spacing, colors, typography
- [ ] Create `theme_extensions.dart` for custom values

### Phase 2: Component Library
- [ ] Create `lib/ui/components/` directory
- [ ] Build `metric_tile.dart` - reusable data display
- [ ] Build `info_panel.dart` - container for metrics
- [ ] Build `control_slider.dart` - engine parameter control
- [ ] Build `status_indicator.dart` - state visualization

### Phase 3: Refactor Existing UI
- [ ] Update `main.dart` to use new theme
- [ ] Refactor `monitoring_overlay.dart` with new components
- [ ] Apply consistent spacing using design tokens
- [ ] Remove hardcoded colors and sizes

### Phase 4: New Features
- [ ] Engine control panel (RPM, load simulation)
- [ ] Performance graph (real-time chart)
- [ ] Settings panel
- [ ] About/help screen

### Phase 5: Polish
- [ ] Smooth animations between states
- [ ] Loading states and skeletons
- [ ] Error states and empty states
- [ ] Dark/light theme toggle (if needed)

## Business Model Considerations

### Ad Integration Zones
1. **Banner Ads:** Bottom of screen, above key controls
2. **Interstitial Ads:** Between simulation sessions
3. **Rewarded Ads:** Unlock advanced features, parts, configurations
4. **Native Ads:** In settings or info panels

### Premium Features Layout
- Clear visual distinction for locked features
- Upgrade CTAs in accessible but non-intrusive locations
- Preview of premium content without full access

## Example Code Structure

```
lib/
├── theme/
│   ├── app_theme.dart           # Main theme configuration
│   ├── design_tokens.dart       # Spacing, colors, sizes
│   └── theme_extensions.dart    # Custom theme data
├── ui/
│   ├── components/
│   │   ├── metric_tile.dart     # Reusable metric display
│   │   ├── info_panel.dart      # Card container
│   │   ├── control_slider.dart  # Parameter control
│   │   └── status_indicator.dart # State display
│   ├── screens/
│   │   ├── engine_screen.dart   # Main simulator view
│   │   └── settings_screen.dart # Configuration
│   └── monitoring_overlay.dart  # Diagnostics (refactored)
└── main.dart                    # App entry with theme
```

## References

- Material Design 3: https://m3.material.io/
- Flutter Theme Guide: https://docs.flutter.dev/cookbook/design/themes
- Engineering UI Best Practices: HMI design standards (ISO 9241)
- Mobile-First Design: Responsive breakpoints and touch targets
- shadcn/ui Philosophy: Component composition over configuration

## Next Steps

1. Review this document with stakeholders
2. Prioritize implementation phases
3. Create proof-of-concept with 2-3 redesigned components
4. Iterate based on user testing
5. Roll out incrementally to production

---

**Document Version:** 1.0  
**Last Updated:** 2025-10-11  
**Author:** CylinderWorks Development Team
