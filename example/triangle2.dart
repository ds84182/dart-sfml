import "package:sfml/sfml.dart" as sf;
import 'package:vector_math/vector_math.dart';
import 'dart:typed_data';

const sf.Attribute attrPosition = const sf.Attribute("attrPosition", 0);
const sf.Attribute attrColor = const sf.Attribute("attrColor", 1);
const List<sf.Attribute> attributes = const [
  attrPosition, attrColor
];

const sf.AttributeFormat attrFmtPos2D = const sf.AttributeFormat(sf.AttributeType.Float, 2);
const sf.AttributeFormat attrFmtColorF4 = const sf.AttributeFormat(sf.AttributeType.Float, 4);

main() async {
  var context = new sf.RenderContext(new sf.VideoMode(800, 600), "dart:sf");

  var shader = new sf.Shader(context);
  await shader.compile(
  """
  attribute vec4 attrPosition;
  attribute vec4 attrColor;

  varying vec4 varyColor;

  void main() {
    varyColor = attrColor;
    gl_Position = attrPosition;
  }
  """,
  """
  varying vec4 varyColor;

  void main() {
    gl_FragColor = varyColor;
  }
  """, attributes);

  var vao = new sf.VertexArray(context);
  var data = new Float32List.fromList([
   0.0, 0.5, 1.0, 0.0, 0.0, 1.0,
   -0.5, -0.5, 0.0, 1.0, 0.0, 1.0,
   0.5, -0.5, 0.0, 0.0, 1.0, 1.0
  ]);
  var buffer = new sf.VertexBuffer(context, data, sf.BufferUsage.StaticDraw);
  vao.bind(attrPosition, buffer, attrFmtPos2D, 6*Float32List.BYTES_PER_ELEMENT);
  vao.bind(attrColor, buffer, attrFmtColorF4, 6*Float32List.BYTES_PER_ELEMENT, 2*Float32List.BYTES_PER_ELEMENT);

  var clearCmd = new sf.ClearCommand(Colors.black);

  var prepareFrame = new sf.CommandBuffer([clearCmd]);

  var shaderBindCmd = new sf.BindShaderCommand(shader);
  var vaoBindCmd = new sf.BindVertexArrayCommand(vao);
  var drawArrayCmd = new sf.DrawArraysCommand(sf.PrimitiveMode.Triangles, 0, 3);

  var drawTriangle = new sf.CommandBuffer([shaderBindCmd, vaoBindCmd, drawArrayCmd]);

  context.setCommands([prepareFrame, drawTriangle]);

  var limiter = new sf.FrameLimiter(60);

  context.frame.listen((_) async {
    await limiter.sync;

    context.render();
  });

  limiter.fpsStream.listen((fps) {
    print("Frame rate: $fps fps");
  });
}
