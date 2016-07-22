part of sfml;

class BindShaderCommand extends Command {
  BindShaderCommand(Shader shader) {
    _init();
    _setShaderRaw(shader._ptr);
  }

  void _init() native "BindShaderCommand";

  void setShader(Shader shader) {
    _setShaderRaw(shader._ptr);
  }

  void _setShaderRaw(int ptr) native "BindShaderCommand::setShader";
}
