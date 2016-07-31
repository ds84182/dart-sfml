part of sfml;

class DrawElementsCommand extends Command {
  DrawElementsCommand(IndexBuffer ibo, PrimitiveMode mode, int count, [int offset = 0]) {
    _init();
    _setData(ibo._ptr, mode.index, offset, count);
  }

  void _init() native "DrawElementsCommand";
  void _setData(int iboptr, int mode, int first, int count) native "DrawElementsCommand::setData";
}
