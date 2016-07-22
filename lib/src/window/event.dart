part of sfml;

abstract class Event {
  factory Event._close() => new CloseEvent._();
  factory Event._resize(int width, int height) => new ResizeEvent._(width, height);
  factory Event._keyPress(int code, bool alt, bool control, bool shift, bool system) =>
    new KeyPressEvent._(
      code < 0 || code >= KeyCode.values.length ? KeyCode.Unknown : KeyCode.values[code], alt,
      control, shift, system);
  factory Event._keyRelease(int code, bool alt, bool control, bool shift, bool system) =>
    new KeyReleaseEvent._(
      code < 0 || code >= KeyCode.values.length ? KeyCode.Unknown : KeyCode.values[code], alt,
      control, shift, system);
  factory Event._textEnter(int unicode) => new TextEnterEvent._(new String.fromCharCode(unicode));
  factory Event._mouseMove(int x, int y) => new MouseMoveEvent._(x, y);
  factory Event._mousePress(int button, int x, int y) =>
    new MouseButtonPressEvent._(MouseButton.values[button], x, y);
  factory Event._mouseRelease(int button, int x, int y) =>
    new MouseButtonReleaseEvent._(MouseButton.values[button], x, y);
  factory Event._unknown(int type) =>
    new UnknownEvent._(EventType.values[type]);
}

class CloseEvent implements Event {
  CloseEvent._();
}

class ResizeEvent implements Event {
  final int width, height;
  ResizeEvent._(this.width, this.height);
}

class KeyPressEvent implements Event {
  final KeyCode key;
  final bool alt, control, shift, system;
  KeyPressEvent._(this.key, this.alt, this.control, this.shift, this.system);
}

class KeyReleaseEvent implements Event {
  final KeyCode key;
  final bool alt, control, shift, system;
  KeyReleaseEvent._(this.key, this.alt, this.control, this.shift, this.system);
}

class TextEnterEvent implements Event {
  final String char;
  TextEnterEvent._(this.char);
}

class MouseMoveEvent implements Event {
  final int x, y;
  MouseMoveEvent._(this.x, this.y);
}

class MouseButtonPressEvent implements Event {
  final MouseButton button;
  final int x, y;
  MouseButtonPressEvent._(this.button, this.x, this.y);
}

class MouseButtonReleaseEvent implements Event {
  final MouseButton button;
  final int x, y;
  MouseButtonReleaseEvent._(this.button, this.x, this.y);
}

enum EventType {
  Closed, Resized, LostFocus, GainedFocus,
  TextEntered, KeyPressed, KeyReleased, MouseWheelMoved,
  MouseWheelScrolled, MouseButtonPressed, MouseButtonReleased, MouseMoved,
  MouseEntered, MouseLeft, JoystickButtonPressed, JoystickButtonReleased,
  JoystickMoved, JoystickConnected, JoystickDisconnected, TouchBegan,
  TouchMoved, TouchEnded, SensorChanged
}

class UnknownEvent implements Event {
  final EventType type;

  UnknownEvent._(this.type);

  @override
  String toString() => "UnknownEvent: $type";
}
