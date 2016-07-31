part of sfml;

class SetUniformCommand extends Command {
  SetUniformCommand(Uniform uniform, data, [int count = 1]) {
    _init();
    update(uniform, data, count);
  }

  void _init() native "SetUniformCommand";
  void _update(String name, int type, TypedData data, int count) native "SetUniformCommand::update";

  void update(Uniform uniform, data, [int count = 1]) {
    _update(uniform.name, uniform.type.index, _coerceData(data), 1);
  }

  static TypedData _coerceData(data) {
    if (data is TypedData) {
      return data;
    } else {
      // Blindly access ".storage" for compat with vector_math
      // TODO: Create typed data from Lists that contain all of one type
      return data.storage; // Cross your fingers!
    }
  }
}
