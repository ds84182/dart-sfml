part of sfml;

class VideoMode {
  final int width;
  final int height;
  final int bitsPerPixel;

  const VideoMode(this.width, this.height, [this.bitsPerPixel = 32]);

  bool get valid native "VideoMode::isValid";

  static VideoMode get desktopMode native "VideoMode::getDesktopMode";
  static List<VideoMode> get fullscreenModes native "VideoMode::getFullscreenModes";
}

// TODO: Make this abstract
abstract class Window {
  void display() native "Window::display";
  bool get isOpen native "Window::isOpen";
  void close() native "Window::close";
  Event pollEvent() native "Window::pollEvent";
}
