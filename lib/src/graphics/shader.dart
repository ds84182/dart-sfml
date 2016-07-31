part of sfml;

class ShaderError extends Error {
  final String message;

  ShaderError(this.message);

  @override
  String toString() => "ShaderError: $message";
}

class Shader extends NativeObject {
  RenderContext _context;
  bool _compiled = false;

  bool get compiled => _compiled;

  Shader(this._context) {_init();}

  void _init() native "Shader";

  Future compile(String vs, String fs, [List<Attribute> attrLoc]) {
    if (_compiled) throw new ShaderError("Shader already compiled");
    return _context.compile(this, vs, fs, attrLoc).then((_) {
      _compiled = true;
    });
  }
}

class Attribute {
  final String name;
  final int index;

  const Attribute(this.name, this.index);
}

enum AttributeType {
  Byte, UnsignedByte, Short, UnsignedShort,
  Int, UnsignedInt, Float
}

class AttributeFormat {
  final AttributeType type;
  final int size;
  final bool normalized;

  const AttributeFormat(this.type, this.size, [this.normalized = false]);
}

enum UniformType {
  Float, Float2, Float3, Float4,
  Int, Int2, Int3, Int4,
  Matrix2, Matrix3, Matrix4
}

class Uniform {
  final String name;
  final UniformType type;
  final int length;

  const Uniform(this.name, this.type, [this.length = 1]);
}
