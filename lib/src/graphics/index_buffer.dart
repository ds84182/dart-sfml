part of sfml;

enum _IndexBufferElementType {
  Int, Short, Byte
}

class IndexBuffer extends NativeObject {
  RenderContext _context;
  final BufferUsage usage;
  IndexBuffer(this._context, TypedData data, this.usage) {
    _IndexBufferElementType elemType;
    if (data is Int32List || data is Uint32List) {
      elemType = _IndexBufferElementType.Int;
    } else if (data is Int16List || data is Uint16List) {
      elemType = _IndexBufferElementType.Short;
    } else if (data is Int8List || data is Uint8List) {
      elemType = _IndexBufferElementType.Byte;
    } else {
      throw new ArgumentError.value(data, "data", "Expected Uint[8|16|32]List or Int[8|16|32]List");
    }
    _init(_context._ptr, data, elemType.index, usage.index);
  }

  void _init(int context, TypedData data, int type, int usage) native "IndexBuffer";

  void update(TypedData data, [int offset = 0]) native "IndexBuffer::update";
}
