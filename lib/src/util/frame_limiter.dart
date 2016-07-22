part of sfml;

class FrameLimiter {
  final int framerate;

  final Stopwatch _stopwatch = new Stopwatch();
  Duration _frameDuration;
  Duration get frameDuration => _frameDuration;

  int _framecount = 0;
  int _fps = 0;
  int get framecount => framecount;
  int get fps => _fps;
  StreamController<int> _fpsStreamCtrl = new StreamController.broadcast();
  Stream<int> get fpsStream => _fpsStreamCtrl.stream;

  Timer _resetTimer;

  FrameLimiter(this.framerate, [bool resetFrameCount = true]) {
    _frameDuration = const Duration(seconds: 1)~/framerate;
    if (resetFrameCount) {
      _resetTimer = new Timer.periodic(const Duration(seconds: 1), (_) {
        _fps = _framecount;
        _framecount = 0;
        _fpsStreamCtrl.add(_fps);
      });
    }
    _stopwatch.start();
  }

  void stop() {
    if (_resetTimer != null) _resetTimer.cancel();
    _stopwatch.stop();
  }

  void reset() {
    _framecount = 0;
  }

  Future<double> get sync {
    Duration delta = _stopwatch.elapsed;
    if (delta > _frameDuration) {
      double dt = _stopwatch.elapsedTicks/_stopwatch.frequency;
      _stopwatch..reset()..start();
      _framecount++;
      return new Future.value(dt);
    } else {
      return new Future.delayed(_frameDuration-delta, () {
        double dt = _stopwatch.elapsedTicks/_stopwatch.frequency;
        _stopwatch..reset()..start();
        _framecount++;
        return dt;
      });
    }
  }
}
