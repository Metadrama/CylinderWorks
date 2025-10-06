import 'dart:convert';
import 'dart:io';
import 'dart:math' as math;

import 'package:xml/xml.dart';

void main(List<String> args) {
  bool zeroRotations = false;
  final positional = <String>[];
  for (final arg in args) {
    if (arg == '--no-rotation') {
      zeroRotations = true;
    } else {
      positional.add(arg);
    }
  }

  final inputPath =
      positional.isNotEmpty ? positional[0] : 'assets/engine/Assem1.xml';
  final outputPath = positional.length > 1
      ? positional[1]
      : 'assets/engine/assembly.json';

  final inputFile = File(inputPath);
  if (!inputFile.existsSync()) {
    stderr.writeln('Input XML not found: $inputPath');
    exitCode = 2;
    return;
  }

  final document = XmlDocument.parse(inputFile.readAsStringSync());
  final namespace = document.rootElement.getAttribute('xmlns') ?? '';
  if (namespace.isEmpty) {
    stderr.writeln('Missing default namespace in XML document.');
    exitCode = 2;
    return;
  }

  final partInfos = _parseParts(document, namespace);
  if (partInfos.isEmpty) {
    stderr.writeln('No part definitions were found in the XML input.');
    exitCode = 2;
    return;
  }

  final meshLookup =
      _scanMeshDirectory('assets/engine/parts', 'engine/parts');
  if (meshLookup.isEmpty) {
    stderr.writeln('No mesh assets found in assets/engine/parts.');
    exitCode = 2;
    return;
  }

  XmlElement? instanceTree;
  final treeIterator =
      document.findAllElements('InstanceTree', namespace: namespace).iterator;
  if (treeIterator.moveNext()) {
    instanceTree = treeIterator.current;
  }
  if (instanceTree == null) {
    stderr.writeln('InstanceTree element not found in XML.');
    exitCode = 2;
    return;
  }

  final parts = <_PartInstance>[];
  for (final instance
      in instanceTree.findElements('Instance', namespace: namespace)) {
    _traverseInstance(
      instance,
      _Transform.identity(),
      parts,
      partInfos,
      namespace,
      zeroRotations,
    );
  }

  if (parts.isEmpty) {
    stderr.writeln('No part instances extracted from the XML.');
    exitCode = 2;
    return;
  }

  final counts = <String, int>{};
  final outputParts = <Map<String, dynamic>>[];

  for (final part in parts) {
    final meshPath = meshLookup[part.sanitizedBase];
    if (meshPath == null) {
      stderr.writeln(
          'Skipping instance "${part.instanceName}"; no mesh found for base "${part.sanitizedBase}".');
      continue;
    }

    final count = counts.update(part.sanitizedBase, (value) => value + 1,
        ifAbsent: () => 0);
    final name = count == 0
        ? part.sanitizedBase
        : '${part.sanitizedBase}_${count + 1}';

    outputParts.add({
      'name': name,
      'mesh': meshPath,
      'anchor': {
        'position': part.position,
        'rotationEuler': part.rotationEuler,
        'color': part.color,
      },
    });
  }

  if (outputParts.isEmpty) {
    stderr.writeln('No parts with matching meshes were generated.');
    exitCode = 3;
    return;
  }

  final encoder = JsonEncoder.withIndent('  ');
  final outputJson = encoder.convert({'parts': outputParts});
  File(outputPath).writeAsStringSync('$outputJson\n');
  stdout.writeln(
      'Wrote ${outputParts.length} part entries derived from ${parts.length} instances to $outputPath');
}

Map<String, _PartInfo> _parseParts(XmlDocument document, String namespace) {
  final result = <String, _PartInfo>{};
  final iterator =
      document.findAllElements('Parts', namespace: namespace).iterator;
  if (!iterator.moveNext()) {
    return result;
  }
  final partsElement = iterator.current;

  for (final element
      in partsElement.findElements('Part', namespace: namespace)) {
    final uid = element.getAttribute('uid');
    if (uid == null || uid.isEmpty) {
      continue;
    }
    final name = element.getAttribute('name') ?? uid;
    final color = _extractDiffuse(element, namespace);
    result[uid] = _PartInfo(
      uid: uid,
      displayName: name,
      sanitizedBase: _sanitizeName(name),
      color: color,
    );
  }

  return result;
}

Map<String, String> _scanMeshDirectory(
  String path,
  String assetRelativePrefix,
) {
  final directory = Directory(path);
  if (!directory.existsSync()) {
    return {};
  }

  final result = <String, String>{};
  for (final entity in directory.listSync(recursive: false, followLinks: false)) {
    if (entity is! File) {
      continue;
    }
    final lowerPath = entity.path.toLowerCase();
    if (!lowerPath.endsWith('.glb')) {
      continue;
    }
    final segments = entity.path.split(Platform.pathSeparator);
    final fileName = segments.isNotEmpty ? segments.last : entity.path;
    final baseName = fileName.substring(0, fileName.length - 4);
    final sanitized = _sanitizeName(baseName);
    final relativePath = assetRelativePrefix.isEmpty
        ? fileName
        : '$assetRelativePrefix/$fileName';
    result[sanitized] = relativePath;
  }
  return result;
}

void _traverseInstance(
  XmlElement element,
  _Transform parent,
  List<_PartInstance> out,
  Map<String, _PartInfo> parts,
  String namespace,
  bool zeroRotations,
) {
  final localTransform = _parseTransform(element, namespace);
  final worldTransform = parent.multiplied(localTransform);

  final entityUid = element.getAttribute('entityUid');
  if (entityUid != null) {
    final partInfo = parts[entityUid];
    if (partInfo != null) {
      out.add(_PartInstance(
        sanitizedBase: partInfo.sanitizedBase,
        instanceName: element.getAttribute('name') ?? partInfo.displayName,
        position: _roundVector(worldTransform.translation),
        rotationEuler: zeroRotations
            ? <double>[0.0, 0.0, 0.0]
            : _rotationToEuler(worldTransform.rotation),
        color: List<double>.from(partInfo.color, growable: false),
      ));
    }
  }

  for (final child in element.findElements('Instance', namespace: namespace)) {
    _traverseInstance(
        child, worldTransform, out, parts, namespace, zeroRotations);
  }
}

_Transform _parseTransform(XmlElement element, String namespace) {
  final transformElement =
      element.getElement('Transform', namespace: namespace);
  if (transformElement == null) {
    return _Transform.identity();
  }

  final rotationElement =
      transformElement.getElement('Rotation', namespace: namespace);
  final translationElement =
      transformElement.getElement('Translation', namespace: namespace);

  final rotation = rotationElement != null
      ? _parseRotation(rotationElement.text)
      : _Transform.identity().rotation;
  final translation = translationElement != null
      ? _parseTranslation(translationElement.text)
      : _Transform.identity().translation;
  return _Transform(rotation: rotation, translation: translation);
}

List<double> _parseRotation(String raw) {
  final values = _parseNumericList(raw, expected: 9);
  if (values.length == 9) {
    return values;
  }
  return _Transform.identity().rotation;
}

List<double> _parseTranslation(String raw) {
  final values = _parseNumericList(raw, expected: 3);
  if (values.length == 3) {
    return values;
  }
  return _Transform.identity().translation;
}

List<double> _parseNumericList(String raw, {required int expected}) {
  final parts = raw.trim().split(RegExp(r'\s+'));
  final result = <double>[];
  for (final part in parts) {
    if (part.isEmpty) {
      continue;
    }
    final value = double.tryParse(part);
    if (value != null && value.isFinite) {
      result.add(value);
    }
    if (result.length == expected) {
      break;
    }
  }
  return result;
}

List<double> _roundVector(List<double> input) {
  return input.map(_roundScalar).toList(growable: false);
}

List<double> _rotationToEuler(List<double> rotation) {
  if (rotation.length != 9) {
    return const [0.0, 0.0, 0.0];
  }

  const double epsilon = 1e-6;
  final r00 = rotation[0];
  final r01 = rotation[1];
  final r02 = rotation[2];
  final r10 = rotation[3];
  final r11 = rotation[4];
  final r12 = rotation[5];
  final r22 = rotation[8];

  double x;
  double y;
  double z;

  if ((r02 >= -1.0 + epsilon) && (r02 <= 1.0 - epsilon)) {
    y = math.asin(-r02);
    x = math.atan2(r12, r22);
    z = math.atan2(r01, r00);
  } else {
    y = r02 < 0.0 ? math.pi / 2.0 : -math.pi / 2.0;
    x = r02 < 0.0 ? math.atan2(r10, r11) : math.atan2(-r10, r11);
    z = 0.0;
  }

  const double radToDeg = 180.0 / math.pi;
  return [
    _roundScalar(x * radToDeg),
    _roundScalar(y * radToDeg),
    _roundScalar(z * radToDeg),
  ];
}

double _roundScalar(double value) {
  const double scale = 1e6;
  final rounded = (value * scale).roundToDouble() / scale;
  return rounded.abs() < 1e-9 ? 0.0 : rounded;
}

List<double> _extractDiffuse(XmlElement element, String namespace) {
  final visual =
      element.getElement('VisualProperties', namespace: namespace);
  final diffuse = visual?.getElement('Diffuse', namespace: namespace);
  final r = double.tryParse(diffuse?.getAttribute('r') ?? '') ?? 0.75;
  final g = double.tryParse(diffuse?.getAttribute('g') ?? '') ?? 0.75;
  final b = double.tryParse(diffuse?.getAttribute('b') ?? '') ?? 0.75;
  return <double>[_roundScalar(r), _roundScalar(g), _roundScalar(b)];
}

String _sanitizeName(String raw) {
  final buffer = StringBuffer();
  bool lastWasUnderscore = false;
  for (final codeUnit in raw.codeUnits) {
    final ch = String.fromCharCode(codeUnit);
    final lower = ch.toLowerCase();
    if (_isAlphanumeric(lower)) {
      buffer.write(lower);
      lastWasUnderscore = false;
    } else {
      if (!lastWasUnderscore && buffer.isNotEmpty) {
        buffer.write('_');
        lastWasUnderscore = true;
      }
    }
  }
  var result = buffer.toString();
  while (result.endsWith('_')) {
    result = result.substring(0, result.length - 1);
  }
  result = result.replaceAll(RegExp(r'_+'), '_');
  if (result.isEmpty) {
    return 'part';
  }
  return result;
}

bool _isAlphanumeric(String character) {
  if (character.isEmpty) {
    return false;
  }
  final code = character.codeUnitAt(0);
  final isDigit = code >= 0x30 && code <= 0x39;
  final isLower = code >= 0x61 && code <= 0x7a;
  return isDigit || isLower;
}

class _Transform {
  _Transform({required this.rotation, required this.translation});

  final List<double> rotation; // Row-major 3x3 matrix.
  final List<double> translation; // XYZ vector in meters.

  factory _Transform.identity() => _Transform(
        rotation: <double>[
          1.0, 0.0, 0.0,
          0.0, 1.0, 0.0,
          0.0, 0.0, 1.0,
        ],
        translation: <double>[0.0, 0.0, 0.0],
      );

  _Transform multiplied(_Transform other) {
    final nextRotation = List<double>.filled(9, 0.0);
    for (var row = 0; row < 3; row++) {
      for (var col = 0; col < 3; col++) {
        double sum = 0.0;
        for (var k = 0; k < 3; k++) {
          sum += rotation[row * 3 + k] * other.rotation[k * 3 + col];
        }
        nextRotation[row * 3 + col] = sum;
      }
    }

    final rotated = List<double>.filled(3, 0.0);
    for (var row = 0; row < 3; row++) {
      rotated[row] = rotation[row * 3 + 0] * other.translation[0] +
          rotation[row * 3 + 1] * other.translation[1] +
          rotation[row * 3 + 2] * other.translation[2];
    }

    return _Transform(
      rotation: nextRotation,
      translation: <double>[
        rotated[0] + translation[0],
        rotated[1] + translation[1],
        rotated[2] + translation[2],
      ],
    );
  }
}

class _PartInfo {
  _PartInfo({
    required this.uid,
    required this.displayName,
    required this.sanitizedBase,
    required this.color,
  });

  final String uid;
  final String displayName;
  final String sanitizedBase;
  final List<double> color;
}

class _PartInstance {
  _PartInstance({
    required this.sanitizedBase,
    required this.instanceName,
    required this.position,
    required this.rotationEuler,
    required this.color,
  });

  final String sanitizedBase;
  final String instanceName;
  final List<double> position;
  final List<double> rotationEuler;
  final List<double> color;
}
