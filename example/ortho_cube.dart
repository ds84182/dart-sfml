import 'dart:math' as math;
import 'dart:typed_data';

import "package:sfml/sfml.dart" as sf;
import 'package:vector_math/vector_math.dart';

const sf.Attribute attrPosition = const sf.Attribute("attrPosition", 0);
const sf.Attribute attrColor = const sf.Attribute("attrColor", 1);
const List<sf.Attribute> attributes = const [
  attrPosition, attrColor
];

const sf.AttributeFormat attrFmtPos3D = const sf.AttributeFormat(sf.AttributeType.Float, 3);
const sf.AttributeFormat attrFmtColorF4 = const sf.AttributeFormat(sf.AttributeType.Float, 4);

const sf.Uniform unifProjection = const sf.Uniform("unifProjection", sf.UniformType.Matrix4);
const sf.Uniform unifModel = const sf.Uniform("unifModel", sf.UniformType.Matrix4);

main() async {
  var context = new sf.RenderContext(new sf.VideoMode(600, 600), "dart:sf");

  var shader = new sf.Shader(context);
  await shader.compile(
  """
  attribute vec4 attrPosition;
  attribute vec4 attrColor;

  uniform mat4 unifProjection;
  uniform mat4 unifModel;

  varying vec4 varyColor;

  void main() {
    varyColor = attrColor;
    gl_Position = unifProjection*unifModel*attrPosition;
  }
  """,
  """
  varying vec4 varyColor;

  void main() {
    gl_FragColor = varyColor;
  }
  """, attributes);

  const double NX = -0.5, NY = NX, NZ = NX;
  const double PX = 0.5, PY = PX, PZ = PX;

  // TODO: sf.ElementBuffer, implemented like sf.VertexBuffer
  var vao = new sf.VertexArray(context);
  var data = new Float32List.fromList([
    NX, NY, NZ, 0.0, 0.0, 0.0, 1.0,
    PX, NY, NZ, 1.0, 0.0, 0.0, 1.0,
    NX, PY, NZ, 0.0, 1.0, 0.0, 1.0,
    PX, PY, NZ, 1.0, 1.0, 0.0, 1.0,

    NX, NY, PZ, 0.0, 0.0, 1.0, 1.0,
    PX, NY, PZ, 1.0, 0.0, 1.0, 1.0,
    NX, PY, PZ, 0.0, 1.0, 1.0, 1.0,
    PX, PY, PZ, 1.0, 1.0, 1.0, 1.0,
  ]);
  var buffer = new sf.VertexBuffer(context, data, sf.BufferUsage.StaticDraw);
  vao.bind(attrPosition, buffer, attrFmtPos3D, 7*Float32List.BYTES_PER_ELEMENT);
  vao.bind(attrColor, buffer, attrFmtColorF4, 7*Float32List.BYTES_PER_ELEMENT, 3*Float32List.BYTES_PER_ELEMENT);

  // The type used in OpenGL is inferred from TypedData's type
  var indicies = new Uint8List.fromList([
    0, 1, 2,  2, 1, 3,
    4, 5, 6,  6, 5, 7,

    0, 4, 6,  0, 6, 2,
    1, 5, 7,  1, 7, 3,

    0, 4, 5,  0, 5, 1,
    2, 6, 7,  2, 7, 3,
  ]);
  var ibo = new sf.IndexBuffer(context, indicies, sf.BufferUsage.StaticDraw);

  var projectionMatrix = makeOrthographicMatrix(0.0, 8.0, 8.0, 0.0, -1.0, 1000.0);
  var modelMatrix = new Matrix4.identity()
    ..translate(4.0, 4.0);

  var enableDepthCmd = new sf.EnableCapabilityCommand(sf.RenderCapability.DepthTest);
  var shaderBindCmd = new sf.BindShaderCommand(shader);
  var setProjectionCmd = new sf.SetUniformCommand(unifProjection, projectionMatrix);
  var clearCmd = new sf.ClearCommand(Colors.black, sf.BufferMask.Color|sf.BufferMask.Depth);

  var prepareFrame = new sf.CommandBuffer([enableDepthCmd, clearCmd, shaderBindCmd, setProjectionCmd]);

  var setModelCmd = new sf.SetUniformCommand(unifModel, modelMatrix);
  var vaoBindCmd = new sf.BindVertexArrayCommand(vao);
  var drawElementsCmd = new sf.DrawElementsCommand(ibo, sf.PrimitiveMode.Triangles, indicies.length);

  var drawCube = new sf.CommandBuffer([shaderBindCmd, setModelCmd, vaoBindCmd, drawElementsCmd]);

  context.setCommands([prepareFrame, drawCube]);

  var limiter = new sf.FrameLimiter(60);

  context.frame.listen((_) async {
    double dt = await limiter.sync;

    context.render();

    dt *= 0.5;
    modelMatrix
      ..rotateX(math.PI*dt)
      ..rotateY(math.PI*dt*1.5)
      ..rotateZ(math.PI*dt*0.5);
    setModelCmd.update(unifModel, modelMatrix);
    setModelCmd.dirty();
  });

  limiter.fpsStream.listen((fps) {
    print("Frame rate: $fps fps");
  });
}
