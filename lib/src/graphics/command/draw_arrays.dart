part of sfml;

enum PrimitiveMode {
  Points, LineStrip, LineLoop, Lines,
  TriangleStrip, TriangleFan, Triangles
}

class DrawArraysCommand extends Command {
  DrawArraysCommand(PrimitiveMode mode, int first, int count) {
    _init();
    _setData(mode.index, first, count);
  }

  void _init() native "DrawArraysCommand";
  void _setData(int mode, int first, int count) native "DrawArraysCommand::setData";
}
