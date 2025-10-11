import 'package:flutter/material.dart';
import 'design_tokens.dart';

/// Custom theme extension for engineering-specific values
@immutable
class EngineeringTheme extends ThemeExtension<EngineeringTheme> {
  const EngineeringTheme({
    required this.dataRpmColor,
    required this.dataTorqueColor,
    required this.dataTemperatureColor,
    required this.dataPressureColor,
    required this.dataEfficiencyColor,
    required this.dataFuelColor,
    required this.metricTileBackground,
    required this.panelBackground,
    required this.overlayBackground,
  });

  final Color dataRpmColor;
  final Color dataTorqueColor;
  final Color dataTemperatureColor;
  final Color dataPressureColor;
  final Color dataEfficiencyColor;
  final Color dataFuelColor;
  final Color metricTileBackground;
  final Color panelBackground;
  final Color overlayBackground;

  @override
  EngineeringTheme copyWith({
    Color? dataRpmColor,
    Color? dataTorqueColor,
    Color? dataTemperatureColor,
    Color? dataPressureColor,
    Color? dataEfficiencyColor,
    Color? dataFuelColor,
    Color? metricTileBackground,
    Color? panelBackground,
    Color? overlayBackground,
  }) {
    return EngineeringTheme(
      dataRpmColor: dataRpmColor ?? this.dataRpmColor,
      dataTorqueColor: dataTorqueColor ?? this.dataTorqueColor,
      dataTemperatureColor: dataTemperatureColor ?? this.dataTemperatureColor,
      dataPressureColor: dataPressureColor ?? this.dataPressureColor,
      dataEfficiencyColor: dataEfficiencyColor ?? this.dataEfficiencyColor,
      dataFuelColor: dataFuelColor ?? this.dataFuelColor,
      metricTileBackground: metricTileBackground ?? this.metricTileBackground,
      panelBackground: panelBackground ?? this.panelBackground,
      overlayBackground: overlayBackground ?? this.overlayBackground,
    );
  }

  @override
  EngineeringTheme lerp(ThemeExtension<EngineeringTheme>? other, double t) {
    if (other is! EngineeringTheme) {
      return this;
    }
    return EngineeringTheme(
      dataRpmColor: Color.lerp(dataRpmColor, other.dataRpmColor, t)!,
      dataTorqueColor: Color.lerp(dataTorqueColor, other.dataTorqueColor, t)!,
      dataTemperatureColor: Color.lerp(dataTemperatureColor, other.dataTemperatureColor, t)!,
      dataPressureColor: Color.lerp(dataPressureColor, other.dataPressureColor, t)!,
      dataEfficiencyColor: Color.lerp(dataEfficiencyColor, other.dataEfficiencyColor, t)!,
      dataFuelColor: Color.lerp(dataFuelColor, other.dataFuelColor, t)!,
      metricTileBackground: Color.lerp(metricTileBackground, other.metricTileBackground, t)!,
      panelBackground: Color.lerp(panelBackground, other.panelBackground, t)!,
      overlayBackground: Color.lerp(overlayBackground, other.overlayBackground, t)!,
    );
  }

  static EngineeringTheme get defaultTheme => const EngineeringTheme(
        dataRpmColor: AppColors.dataRpm,
        dataTorqueColor: AppColors.dataTorque,
        dataTemperatureColor: AppColors.dataTemperature,
        dataPressureColor: AppColors.dataPressure,
        dataEfficiencyColor: AppColors.dataEfficiency,
        dataFuelColor: AppColors.dataFuel,
        metricTileBackground: Color(0x14FFFFFF), // 8% white
        panelBackground: Color(0xE6000000), // 90% black
        overlayBackground: Color(0x59000000), // 35% black
      );
}

/// Helper extension to access engineering theme from BuildContext
extension EngineeringThemeExtension on BuildContext {
  EngineeringTheme get engineeringTheme =>
      Theme.of(this).extension<EngineeringTheme>() ?? EngineeringTheme.defaultTheme;
}
