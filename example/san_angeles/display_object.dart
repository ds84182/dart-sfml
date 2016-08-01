part of san_angeles;

abstract class Drawable {
  List<sf.Command> get commands;
}

const sf.AttributeFormat attrFmtPos2D = const sf.AttributeFormat(sf.AttributeType.Float, 2);
const sf.AttributeFormat attrFmtPos3D = const sf.AttributeFormat(sf.AttributeType.Float, 3);
const sf.AttributeFormat attrFmtColorB4 = const sf.AttributeFormat(sf.AttributeType.UnsignedByte, 4, true);
const sf.AttributeFormat attrFmtNormalF3 = const sf.AttributeFormat(sf.AttributeType.Float, 3);

/// An object that can be displayed to the screen
class DisplayObject extends Drawable {
  ByteBuffer bufferData;
  Float32List vertexArray;
  Uint32List colorArray;
  Float32List normalArray;
  int vertexComponents;
  int count;

  sf.VertexBuffer vbo;
  sf.VertexArray vao;

  List<sf.Command> commands;

  DisplayObject(int vertices, int vertexComponents, bool useNormalArray) {
    this.count = vertices;
    this.vertexComponents = vertexComponents;
    int vertexSize = vertices * vertexComponents;
    int realVertexSize = vertexSize * Float32List.BYTES_PER_ELEMENT;
    int colorSize = vertices;
    int realColorSize = colorSize * Uint32List.BYTES_PER_ELEMENT;
    int normalSize = useNormalArray ? vertices * 3 : 0;
    int realNormalSize = normalSize * Float32List.BYTES_PER_ELEMENT;
    this.bufferData = new Uint8List(realVertexSize + realColorSize + realNormalSize).buffer;
    this.vertexArray = bufferData.asFloat32List(0, vertexSize);
    this.colorArray = bufferData.asUint32List(realVertexSize, colorSize);
    this.normalArray = useNormalArray ?
      bufferData.asFloat32List(realVertexSize+realColorSize, normalSize) : null;
  }

  void finalize() {
    int vertexSize = count * vertexComponents;
    int realVertexSize = vertexSize * Float32List.BYTES_PER_ELEMENT;
    int colorSize = count;
    int realColorSize = colorSize * Uint32List.BYTES_PER_ELEMENT;

    vao = new sf.VertexArray(context);
    vbo = new sf.VertexBuffer(context, bufferData.asByteData(), sf.BufferUsage.StaticDraw);
    vao.bind(attrPosition, vbo, vertexComponents == 2 ? attrFmtPos2D : attrFmtPos3D, 0, 0);
    vao.bind(attrColor, vbo, attrFmtColorB4, 0, realVertexSize);

    if (normalArray != null)
      vao.bind(attrNormal, vbo, attrFmtNormalF3, 0, realVertexSize+realColorSize);

    commands = [
      new sf.BindVertexArrayCommand(vao),
      new sf.DrawArraysCommand(sf.PrimitiveMode.Triangles, 0, count)
    ];
  }
}
