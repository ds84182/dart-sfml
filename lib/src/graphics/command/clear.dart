part of sfml;

class BufferMask {
  BufferMask._();

  static const int Color = 0x1;
  static const int Depth = 0x2;
  static const int Stencil = 0x4;
}

class ClearCommand extends Command {
  ClearCommand(Vector4 color, [int mask = BufferMask.Color]) {
    _init();
    setColor(color);
    setMask(mask);
  }

  void _init() native "ClearCommand";

  void setColor(Vector4 color) {
    _setColorRaw(color.storage);
  }

  void _setColorRaw(Float32List color) native "ClearCommand::setColor";

  void setMask(int mask) native "ClearCommand::setMask";
}
