import 'package:flutter/material.dart';
import '../../theme/design_tokens.dart';
import '../../theme/app_theme.dart';

/// Reusable metric tile component for displaying numeric data
/// Optimized for mechanical engineering metrics (RPM, torque, temperature, etc.)
class MetricTile extends StatelessWidget {
  const MetricTile({
    super.key,
    required this.title,
    required this.value,
    this.unit,
    this.color,
    this.icon,
    this.isLoading = false,
    this.onTap,
  });

  final String title;
  final String value;
  final String? unit;
  final Color? color;
  final IconData? icon;
  final bool isLoading;
  final VoidCallback? onTap;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final accentColor = color ?? theme.colorScheme.primary;

    return Material(
      color: theme.colorScheme.primary.withOpacity(0.08),
      borderRadius: BorderRadius.circular(DesignTokens.radiusMd),
      child: InkWell(
        onTap: onTap,
        borderRadius: BorderRadius.circular(DesignTokens.radiusMd),
        child: Container(
          constraints: const BoxConstraints(
            minWidth: DesignTokens.metricTileWidth,
          ),
          padding: const EdgeInsets.symmetric(
            horizontal: DesignTokens.spaceSm + DesignTokens.spaceXs,
            vertical: DesignTokens.spaceSm,
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: [
              Row(
                children: [
                  if (icon != null) ...[
                    Icon(
                      icon,
                      size: DesignTokens.iconXs,
                      color: accentColor.withOpacity(0.7),
                    ),
                    const SizedBox(width: DesignTokens.spaceXs),
                  ],
                  Expanded(
                    child: Text(
                      title,
                      style: theme.textTheme.labelSmall?.copyWith(
                        fontSize: 11,
                        color: accentColor.withOpacity(0.7),
                      ),
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: DesignTokens.spaceXs),
              if (isLoading)
                SizedBox(
                  height: 20,
                  child: LinearProgressIndicator(
                    backgroundColor: theme.colorScheme.surfaceContainerHighest,
                    color: accentColor.withOpacity(0.5),
                  ),
                )
              else
                Row(
                  crossAxisAlignment: CrossAxisAlignment.baseline,
                  textBaseline: TextBaseline.alphabetic,
                  children: [
                    Flexible(
                      child: Text(
                        value,
                        style: AppTheme.monoMedium(context).copyWith(
                          fontWeight: FontWeight.w600,
                          fontSize: 15,
                        ),
                        maxLines: 1,
                        overflow: TextOverflow.ellipsis,
                      ),
                    ),
                    if (unit != null) ...[
                      const SizedBox(width: DesignTokens.spaceXs),
                      Text(
                        unit!,
                        style: theme.textTheme.labelSmall?.copyWith(
                          fontSize: 10,
                          color: theme.colorScheme.onSurfaceVariant
                              .withOpacity(0.6),
                        ),
                      ),
                    ],
                  ],
                ),
            ],
          ),
        ),
      ),
    );
  }
}
