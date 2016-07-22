part of sfml;

abstract class RenderTarget extends NativeObject {
  void clear(Color color) {
    clearRaw(color.raw);
  }

  void clearRaw(int raw) native "RenderTarget::clear";
}
