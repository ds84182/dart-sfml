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

  // There are two different ways to set a uniform
  // The first way is via Shader.sendUniform
  // The second way is SendUniformCommand
  // Shader.sendUniform is meant to be used once a frame OR LESS
  // SendUniformCommand is meant to be used multiple times inside of a frame
  // Multiple Shader.sendUniforms in the middle of command compilation will ALWAYS take the last one
  void sendUniform(Uniform uniform, data, [int count = 1]) {
    // data can be a List if the target uniform is an array
    // data can be (or the data list can contain) TypedData, vector_math's Vector, vector_math's
    // Matrix[n]
    // This is implemented by checking for TypedData then falling back to blindly accessing .storage
    // .storage SHOULD contain TypedData
    // TODO: Type assertions
    if (uniform.length > 1 && data is List) {
      var newList = data.map(_coerceData).toList(growable: false);
      // TODO: Send via RenderContext
    } else {
      data = _coerceData(data);
      _context._sendUniform(this, uniform.name, uniform.type.index, data, 1);
      // TODO: Send via RenderContext
    }
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
