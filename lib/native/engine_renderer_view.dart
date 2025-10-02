import 'dart:io' show Platform;

import 'package:flutter/foundation.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter/services.dart';

/// Flutter-side wrapper for the native renderer surface.
class EngineRendererView extends StatelessWidget {
  const EngineRendererView({super.key});

  @override
  Widget build(BuildContext context) {
    if (!Platform.isAndroid) {
      return const Center(
        child: Text(
          'Engine renderer view is only available on Android for now.',
          textAlign: TextAlign.center,
        ),
      );
    }

    // The gesture recognizers are handled entirely on the native side.
    final Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers = {};

    return AndroidView(
      viewType: 'engine/renderer',
      layoutDirection: TextDirection.ltr,
      creationParams: null,
      creationParamsCodec: const StandardMessageCodec(),
      gestureRecognizers: gestureRecognizers,
      hitTestBehavior: PlatformViewHitTestBehavior.opaque,
    );
  }
}
