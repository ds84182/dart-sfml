part of sfml;

enum BufferUsage {
  StreamDraw, StreamRead, StreamCopy,
  StaticDraw, StaticRead, StaticCopy,
  DynamicDraw, DynamicRead, DynamicCopy
}

class VertexBuffer extends NativeObject {
  RenderContext _context;
  final BufferUsage usage;
  VertexBuffer(this._context, TypedData data, this.usage) {
    _init(_context._ptr, data, usage.index);
  }

  void _init(int context, TypedData data, int usage) native "VertexBuffer";

  void update(TypedData data, [int offset = 0]) native "VertexBuffer::update";
}
