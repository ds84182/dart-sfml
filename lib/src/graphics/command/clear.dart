part of sfml;

class ClearCommand extends Command {
  ClearCommand(Color color) {
    _init(color.raw);
  }

  void _init(int raw) native "ClearCommand";

  void setColor(Color color) {
    _setColorRaw(color.raw);
  }

  void _setColorRaw(int raw) native "ClearCommand::setColor";
}
