part of sfml;

class FrameLimiter {
  final int framerate;

  final Stopwatch _stopwatch = new Stopwatch();
  Duration _frameDuration;
  bool _unlimited;
  Duration get frameDuration => _frameDuration;

  int _framecount = 0;
  int _fps = 0;
  int get framecount => framecount;
  int get fps => _fps;
  final Stopwatch _fpsStopwatch = new Stopwatch();
  StreamController<int> _fpsStreamCtrl = new StreamController.broadcast();
  Stream<int> get fpsStream => _fpsStreamCtrl.stream;

  FrameLimiter(this.framerate) {
    if (framerate == 0) {
      _unlimited = true;
      _frameDuration = Duration.ZERO;
    } else {
      _frameDuration = const Duration(seconds: 1)~/framerate;
    }
    _stopwatch.start();
    _fpsStopwatch.start();
  }

  void stop() {
    _stopwatch.stop();
    _fpsStopwatch.stop();
  }

  void reset() {
    _framecount = 0;
  }

  void _updateFPS() {
    if (_fpsStopwatch.elapsedMilliseconds > 1000) {
      _fpsStopwatch.reset();
      _fps = _framecount;
      _framecount = 0;
      _fpsStreamCtrl.add(_fps);
    }
  }

  Future<double> get sync {
    Duration delta = _stopwatch.elapsed;
    if (delta > _frameDuration) {
      double dt = _stopwatch.elapsedTicks/_stopwatch.frequency;
      _stopwatch..reset()..start();
      _framecount++;
      _updateFPS();
      return new Future.value(dt);
    } else {
      return new Future.delayed(_frameDuration-delta, () {
        double dt = _stopwatch.elapsedTicks/_stopwatch.frequency;
        _stopwatch..reset()..start();
        _framecount++;
        _updateFPS();
        return dt;
      });
    }
  }
}
