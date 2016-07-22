import "package:sfml/sfml.dart" as sf;

main() async {
  var context = new sf.RenderContext(new sf.VideoMode(800, 600), "dart:sf");

  int col = 0;

  var clearCmd = new sf.ClearCommand(new sf.Color.rgba(col, col, col));
  var commandList = new sf.CommandList(context, [clearCmd]);
  context.setCommands([commandList]);

  var limiter = new sf.FrameLimiter(60);

  context.frame.listen((_) async {
    await limiter.sync;

    clearCmd.setColor(new sf.Color.rgba(col, col, col));
    clearCmd.dirty();
    context.render();

    col++;
    if (col > 255) col = 0;
  });

  limiter.fpsStream.listen((fps) {
    print("Frame rate: $fps fps");
  });
}
