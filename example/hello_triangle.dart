import "package:sfml/sfml.dart" as sf;
import 'dart:typed_data';

const sf.Attribute attrPosition = const sf.Attribute("attrPosition", 0);
const List<sf.Attribute> attributes = const [
  attrPosition
];
const sf.AttributeFormat attrFmtPos2D = const sf.AttributeFormat(sf.AttributeType.Float, 2);

main() async {
  var context = new sf.RenderContext(new sf.VideoMode(800, 600), "dart:sf");

  var shader = new sf.Shader(context);
  await shader.compile(
  """
  attribute vec4 attrPosition;
  void main() {
    gl_Position = attrPosition;
  }
  """,
  """
  void main() {
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  }
  """, attributes);

  var vao = new sf.VertexArray(context);
  var data = new Float32List.fromList([
   0.0, 0.5,
   -0.5, -0.5,
   0.5, -0.5
  ]);
  var buffer = new sf.VertexBuffer(context, data, sf.BufferUsage.StaticDraw);
  vao.bind(attrPosition, buffer, attrFmtPos2D);

  var clearCmd = new sf.ClearCommand(new sf.Color.rgba(0, 0, 0));

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
