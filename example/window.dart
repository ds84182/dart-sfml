import "package:sfml/sfml.dart" as sf;
import 'package:vector_math/vector_math.dart';

main() async {
  var context = new sf.RenderContext(new sf.VideoMode(800, 600), "dart:sf");

  double col = 0.0;

  var clearColor = new Vector4(col, col, col, 1.0);
  var clearCmd = new sf.ClearCommand(clearColor);
  var prepareFrame = new sf.CommandBuffer([clearCmd]);
  context.setCommands([prepareFrame]);

  var limiter = new sf.FrameLimiter(60);

  context.frame.listen((_) async {
    double dt = await limiter.sync;

    col += dt/4.0;
    if (col > 1.0) col = 0.0;

    clearColor.setValues(col, col, col, 1.0);
    clearCmd.setColor(clearColor);
    clearCmd.dirty();
    context.render();
  });

  limiter.fpsStream.listen((fps) {
    print("Frame rate: $fps fps");
  });
}
