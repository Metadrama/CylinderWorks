import 'package:flutter/material.dart';
import '../../theme/design_tokens.dart';

/// Professional information panel container
/// Suitable for diagnostics, settings, and data displays
class InfoPanel extends StatelessWidget {
  const InfoPanel({
    super.key,
    this.title,
    this.icon,
    this.onClose,
    required this.child,
    this.padding,
    this.constraints,
    this.backgroundColor,
    this.elevation,
  });

  final String? title;
  final IconData? icon;
  final VoidCallback? onClose;
  final Widget child;
  final EdgeInsets? padding;
  final BoxConstraints? constraints;
  final Color? backgroundColor;
  final double? elevation;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);

    return Card(
      color: backgroundColor ?? theme.colorScheme.surface.withOpacity(DesignTokens.opacityOverlay),
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(DesignTokens.radiusMd + 2),
      ),
      elevation: elevation ?? DesignTokens.elevationMedium + 2,
      child: ConstrainedBox(
        constraints: constraints ??
            const BoxConstraints(
              minWidth: DesignTokens.panelMinWidth,
              maxWidth: DesignTokens.panelMaxWidth,
            ),
        child: Padding(
          padding: padding ??
              const EdgeInsets.symmetric(
                horizontal: DesignTokens.spaceMd,
                vertical: DesignTokens.spaceSm + DesignTokens.spaceXs,
              ),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              if (title != null || icon != null || onClose != null)
                Padding(
                  padding: const EdgeInsets.only(bottom: DesignTokens.spaceSm),
                  child: Row(
                    children: [
                      if (icon != null) ...[
                        Icon(icon, size: DesignTokens.iconSm),
                        const SizedBox(width: DesignTokens.spaceSm),
                      ],
                      if (title != null)
                        Expanded(
                          child: Text(
                            title!,
                            style: theme.textTheme.titleMedium?.copyWith(
                              fontWeight: FontWeight.w600,
                            ),
                          ),
                        ),
                      if (onClose != null)
                        IconButton(
                          visualDensity: VisualDensity.compact,
                          padding: EdgeInsets.zero,
                          constraints: const BoxConstraints(),
                          icon: Icon(
                            Icons.close,
                            size: DesignTokens.iconSm,
                          ),
                          onPressed: onClose,
                        ),
                    ],
                  ),
                ),
              child,
            ],
          ),
        ),
      ),
    );
  }
}
