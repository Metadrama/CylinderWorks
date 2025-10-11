import 'package:flutter/material.dart';

/// Design tokens for consistent spacing, sizing, and visual properties
/// Based on 8-point grid system for optimal mobile touch targets
class DesignTokens {
  DesignTokens._();

  // Spacing system (8-point grid)
  static const double spaceXs = 4.0;
  static const double spaceSm = 8.0;
  static const double spaceMd = 16.0;
  static const double spaceLg = 24.0;
  static const double spaceXl = 32.0;
  static const double spaceXxl = 48.0;

  // Border radius system
  static const double radiusSm = 8.0;
  static const double radiusMd = 12.0;
  static const double radiusLg = 16.0;
  static const double radiusXl = 20.0;
  static const double radiusFull = 9999.0;

  // Elevation system
  static const double elevationNone = 0.0;
  static const double elevationLow = 2.0;
  static const double elevationMedium = 4.0;
  static const double elevationHigh = 8.0;
  static const double elevationHighest = 16.0;

  // Icon sizes
  static const double iconXs = 16.0;
  static const double iconSm = 18.0;
  static const double iconMd = 24.0;
  static const double iconLg = 32.0;
  static const double iconXl = 48.0;

  // Component dimensions
  static const double minTouchTarget = 48.0;
  static const double buttonHeight = 48.0;
  static const double buttonHeightSmall = 36.0;
  static const double inputHeight = 48.0;

  // Container constraints
  static const double maxContentWidth = 1200.0;
  static const double panelMinWidth = 240.0;
  static const double panelMaxWidth = 320.0;
  static const double metricTileWidth = 110.0;

  // Opacity values
  static const double opacityDisabled = 0.38;
  static const double opacityMuted = 0.6;
  static const double opacitySubtle = 0.8;
  static const double opacityOverlay = 0.9;

  // Animation durations (milliseconds)
  static const int durationFast = 150;
  static const int durationNormal = 250;
  static const int durationSlow = 350;

  // Responsive breakpoints
  static const double breakpointMobile = 0;
  static const double breakpointTablet = 600;
  static const double breakpointDesktop = 900;
}

/// Color palette optimized for mechanical engineering applications
class AppColors {
  AppColors._();

  // Primary colors - Technical blue for precision/engineering
  static const Color primary = Color(0xFF2563EB);
  static const Color primaryLight = Color(0xFF60A5FA);
  static const Color primaryDark = Color(0xFF1E40AF);
  static const Color onPrimary = Color(0xFFFFFFFF);

  // Surface colors - Dark mode optimized
  static const Color surface = Color(0xFF1A1C1E);
  static const Color surfaceVariant = Color(0xFF2D3135);
  static const Color surfaceBright = Color(0xFF3A3E42);
  static const Color onSurface = Color(0xFFE4E6E8);
  static const Color onSurfaceVariant = Color(0xFFB8BABD);

  // Background
  static const Color background = Color(0xFF08090D);
  static const Color backgroundElevated = Color(0xFF1A1C1E);

  // Secondary - Green for success/active states
  static const Color secondary = Color(0xFF10B981);
  static const Color secondaryLight = Color(0xFF34D399);
  static const Color secondaryDark = Color(0xFF059669);
  static const Color onSecondary = Color(0xFFFFFFFF);

  // Tertiary - Amber for warnings/attention
  static const Color tertiary = Color(0xFFF59E0B);
  static const Color tertiaryLight = Color(0xFFFBBF24);
  static const Color tertiaryDark = Color(0xFFD97706);
  static const Color onTertiary = Color(0xFF000000);

  // Semantic colors
  static const Color error = Color(0xFFEF4444);
  static const Color errorLight = Color(0xFFF87171);
  static const Color errorDark = Color(0xFFDC2626);
  static const Color onError = Color(0xFFFFFFFF);

  static const Color success = Color(0xFF10B981);
  static const Color warning = Color(0xFFF59E0B);
  static const Color info = Color(0xFF3B82F6);

  // Data visualization colors (for graphs and metrics)
  static const Color dataRpm = Color(0xFF3B82F6); // Blue
  static const Color dataTorque = Color(0xFF8B5CF6); // Purple
  static const Color dataTemperature = Color(0xFFEF4444); // Red
  static const Color dataPressure = Color(0xFF10B981); // Green
  static const Color dataEfficiency = Color(0xFFF59E0B); // Amber
  static const Color dataFuel = Color(0xFF06B6D4); // Cyan

  // Overlay colors
  static const Color overlay = Color(0xFF000000);
  static const Color scrim = Color(0xFF000000);

  // Border and divider colors
  static const Color border = Color(0xFF3A3E42);
  static const Color divider = Color(0xFF2D3135);
}
