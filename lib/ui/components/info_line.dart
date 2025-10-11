import 'package:flutter/material.dart';
import '../../theme/design_tokens.dart';

/// Info line component for label-value pairs
/// Used in panels to display structured information
class InfoLine extends StatelessWidget {
  const InfoLine({
    super.key,
    required this.label,
    required this.value,
    this.icon,
    this.valueColor,
  });

  final String label;
  final String value;
  final IconData? icon;
  final Color? valueColor;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final baseStyle = theme.textTheme.bodySmall;
    final labelColor = baseStyle?.color?.withOpacity(DesignTokens.opacitySubtle);
    final labelStyle = baseStyle?.copyWith(
      fontWeight: FontWeight.w600,
      color: labelColor,
    );

    return Padding(
      padding: const EdgeInsets.symmetric(vertical: DesignTokens.spaceXs / 2),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          if (icon != null) ...[
            Icon(
              icon,
              size: DesignTokens.iconXs,
              color: labelColor,
            ),
            const SizedBox(width: DesignTokens.spaceXs),
          ],
          Text('$label:', style: labelStyle),
          const SizedBox(width: DesignTokens.spaceXs + DesignTokens.spaceXs / 2),
          Expanded(
            child: Text(
              value,
              style: baseStyle?.copyWith(color: valueColor),
              maxLines: 2,
              overflow: TextOverflow.ellipsis,
            ),
          ),
        ],
      ),
    );
  }
}
