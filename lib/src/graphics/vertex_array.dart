part of sfml;

class VertexArray extends NativeObject {
  RenderContext _context;

  VertexArray(this._context) {
    _init(_context._ptr);
  }

  void _init(int contextptr) native "VertexArray";

  void bind(Attribute attr, VertexBuffer buf, AttributeFormat attrFmt, [int stride = 0, int offset = 0]) {
    _enableAndBind(_context._ptr, attr.index, buf._ptr, attrFmt.type.index, attrFmt.size, attrFmt.normalized, stride, offset);
  }

  void _enableAndBind(int context, int index, int buffer, int type, int size, bool normalized, int stride, int offset) native "VertexArray::enableAndBind";

  // TODO: unbind with defaults
}
