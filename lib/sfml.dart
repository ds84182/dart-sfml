library sfml;

import 'dart:async';
import 'dart:isolate';
import 'dart:typed_data';

import 'dart-ext:sfml';

import 'package:vector_math/vector_math.dart';

////////

part 'src/graphics/color.dart';
part 'src/graphics/render_target.dart';
part 'src/graphics/render_context.dart';
part 'src/graphics/shader.dart';
part 'src/graphics/vertex_array.dart';
part 'src/graphics/vertex_buffer.dart';

part 'src/graphics/command.dart';
part 'src/graphics/command/bind_shader.dart';
part 'src/graphics/command/bind_vertex_array.dart';
part 'src/graphics/command/clear.dart';
part 'src/graphics/command/draw_arrays.dart';
part 'src/graphics/command/render_capability.dart';

part 'src/util/frame_limiter.dart';
part 'src/util/pair.dart';

part 'src/window.dart';
part 'src/window/event.dart';
part 'src/window/keyboard.dart';
part 'src/window/mouse.dart';

abstract class NativeObject {
  int _ptr;

  @override
  String toString() => "$runtimeType: 0x${(_ptr ?? 0).toRadixString(16).padLeft(16, '0')}";

  int get hashCode => _ptr ?? 0;
}
