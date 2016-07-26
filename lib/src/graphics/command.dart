part of sfml;

// Dart side command objects are really data objects that manage an ACTUAL command
// It has the ability to set itself as dirty so that the next render frame will change
class Command extends NativeObject {
  void dirty() native "Command::dirty";
}

class CommandBuffer extends NativeObject {
  CommandBuffer(List<Command> commands) {
    _init();
    update(commands);
  }

  void _init() native "CommandBuffer";
  void update(List<Command> commands) native "CommandBuffer::update";
}
