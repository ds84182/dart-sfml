part of sfml;

enum _RenderThreadMessageType {
  Init, // To Render Thread
  StartFrame, // To Dart Thread
  RenderFrame, // To Render Thread
  SetCommands, // To Render Thread (Format of [RenderTarget, [CommandPtr, ...], ...])
  NewShader, // To Render Thread ([ptr, vertexShaderStr, fragmentShaderStr, attributeLayout]). We get a reply back with the ptr and the status
  ShaderResult, // To Dart Thread (ptr, errorLog as string if failed or null
}

class RenderContext extends RenderTarget with Window {
  StreamController _frameController;
  Stream get frame => _frameController.stream;

  SendPort _port;
  ReceivePort _recv = new ReceivePort();
  bool _ready = true;
  Map<int, Completer> _shadersWaitingResult = {};

  RenderContext(VideoMode mode, String title) {
    _init(mode, title);

    _frameController = new StreamController(onListen: () {
      if (_ready) {
        _frameController.add(null);
      }
    });

    _recv.listen((message) {
      if (message == _RenderThreadMessageType.StartFrame.index) {
        _ready = true;
        _frameController.add(null);
      } else if (message is List && message[0] == _RenderThreadMessageType.ShaderResult.index) {
        var error = message[2];
        var completer = _shadersWaitingResult[message[1]];
        if (error != null) {
          completer.completeError(new ShaderError(error.trim()));
        } else {
          completer.complete();
        }

        _shadersWaitingResult.remove(message[1]);
      } else {
        print("Unknown message received: $message");
      }
    });
    _port.send([_ptr, _RenderThreadMessageType.Init.index, _recv.sendPort]);
  }

  void _init(VideoMode mode, String title) native "RenderContext";

  void render() {
    _checkReady();
    _ready = false;
    _port.send([_ptr, _RenderThreadMessageType.RenderFrame.index]);
  }

  void setCommands(List<CommandList> list) {
    _checkReady();
    var nativeList = list
      .expand((cl) => [cl.target._ptr, cl.commands.map((cmd) => cmd._ptr).toList(growable: false)])
      .toList(growable: false);
    _port.send([_ptr, _RenderThreadMessageType.SetCommands.index, nativeList]);
  }

  void _checkReady() {
    if (!_ready) throw new StateError("Window not ready for render");
  }

  Future compile(Shader shader, String vs, String fs, [List<Attribute> attrLoc]) {
    _checkReady();

    var completer = new Completer();
    var attrLocList;

    if (attrLoc != null) {
      attrLocList = [];
      for (var attr in attrLoc) {
        attrLocList..add(attr.name)..add(attr.index);
      }
    }

    _shadersWaitingResult[shader._ptr] = completer;
    _port.send([_ptr, _RenderThreadMessageType.NewShader.index, shader._ptr, vs, fs, attrLocList]);
    return completer.future;
  }
}
