part of sfml;

class BindVertexArrayCommand extends Command {
  BindVertexArrayCommand(VertexArray vao) {
    _init();
    _setVertexArray(vao._ptr);
  }

  void _init() native "BindVertexArrayCommand";

  void _setVertexArray(int vaoptr) native "BindVertexArrayCommand::setVertexArray";
}
