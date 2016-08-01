library san_angeles;

import 'dart:math' as math;
import 'dart:typed_data';

import "package:sfml/sfml.dart" as sf;
import 'package:vector_math/vector_math.dart';

part 'san_angeles/cameras.dart';
part 'san_angeles/display_object.dart';
part 'san_angeles/random.dart';
part 'san_angeles/shader.dart';
part 'san_angeles/shapes.dart';

sf.RenderContext context = new sf.RenderContext(new sf.VideoMode(600, 600), "dart:sf");
int tick = 0;
int startTick;

num degToRad(num deg) => deg * (math.PI / 180.0);

main() async {
  var shader = new sf.Shader(context);
  await shader.compile(LIT_VERTEX_SRC, COMMON_FRAGMENT_SRC, attributes);

  var projectionMatrix = makePerspectiveMatrix(degToRad(45.0), 600.0/600.0, 0.5, 150.0);

  var viewMatrix = new Matrix4.identity();

  ///

  var enableDepthCmd = new sf.EnableCapabilityCommand(sf.RenderCapability.DepthTest);
  var shaderBindCmd = new sf.BindShaderCommand(shader);
  var clearCmd = new sf.ClearCommand(new Vector4(.1,.2,.3,1.0), sf.BufferMask.Color|sf.BufferMask.Depth);

  var L0BaseDirection = new Vector3(-4.0, 1.0, 1.0);
  var L1BaseDirection = new Vector3(1.0, -2.0, -1.0);
  var L2BaseDirection = new Vector3(-1.0, 0.0, -4.0);
  var L0Direction, L1Direction, L2Direction;

  var lightUnifCmds = [
    new sf.SetUniformCommand(unifAmbient, new Vector4(0.2,0.2,0.2,1.0)),
    new sf.SetUniformCommand(unifL0Diffuse, new Vector4(1.0,0.4,0.0,1.0)),
    L0Direction = new sf.SetUniformCommand(unifL0Direction, L0BaseDirection),
    new sf.SetUniformCommand(unifL1Diffuse, new Vector4(0.07,0.14,0.35,1.0)),
    L1Direction = new sf.SetUniformCommand(unifL1Direction, L1BaseDirection),
    new sf.SetUniformCommand(unifL2Diffuse, new Vector4(0.07,0.17,0.14,1.0)),
    L2Direction = new sf.SetUniformCommand(unifL2Direction, L2BaseDirection),
    new sf.SetUniformCommand(unifL0Specular, new Vector4(1.0,1.0,1.0,1.0)),
    new sf.SetUniformCommand(unifShininess, new Float32List.fromList([60.0])),
  ];

  var prepareFrame = new sf.CommandBuffer([enableDepthCmd, clearCmd, shaderBindCmd]..addAll(lightUnifCmds));

  ///

  randomSeed = 15;
  var shapeObjects = superShapeParams.map(createSuperShape).toList(growable: false) as List<DisplayObject>;

  randomSeed = 9;
  const double translationScale = 9.0;

  var buildings = [] as List<Building>;
  var flippedBuildings = [] as List<Building>;

  for (int y = -5; y <= 5; y++) {
    for (int x = -5; x <= 5; x++) {
      int curShape = randomUInt() % superShapeParams.length;
      num rot = degToRad(randomUInt() % 360);
      buildings.add(new Building(curShape, shapeObjects[curShape], x * translationScale, y * translationScale, rot));
      flippedBuildings.add(new Building(curShape, shapeObjects[curShape], x * translationScale, y * translationScale, rot, flipped: true));
    }
  }

  var drawBuildings = new sf.CommandBuffer([shaderBindCmd]
    ..addAll(buildings.expand((b) => b.commands)));
  var drawFlippedBuildings = new sf.CommandBuffer([shaderBindCmd]
    ..addAll(flippedBuildings.expand((b) => b.commands)));

  context.setCommands([prepareFrame, drawBuildings, drawFlippedBuildings]);

  var limiter = new sf.FrameLimiter(60);

  double time = 0.0;

  context.frame.listen((_) async {
    double dt = await limiter.sync;
    int time2tick = (time * 1000).toInt();
    if (startTick == null) {
      startTick = time2tick;
    }
    tick = (tick + time2tick - startTick) >> 1;
    time += dt;

    camTrack(viewMatrix);

    Vector3 transform3(Vector3 v) {
      final argStorage = v.storage;
      final mStorage = viewMatrix.storage;
      final x_ = (mStorage[0] * argStorage[0]) +
          (mStorage[4] * argStorage[1]) +
          (mStorage[8] * argStorage[2]);
      final y_ = (mStorage[1] * argStorage[0]) +
          (mStorage[5] * argStorage[1]) +
          (mStorage[9] * argStorage[2]);
      final z_ = (mStorage[2] * argStorage[0]) +
          (mStorage[6] * argStorage[1]) +
          (mStorage[10] * argStorage[2]);
      argStorage[0] = x_;
      argStorage[1] = y_;
      argStorage[2] = z_;
      return v;
    }

    var vc3 = new Vector3.copy(L0BaseDirection);
    L0Direction.update(unifL0Direction, transform3(vc3));
    L0Direction.dirty();
    L1BaseDirection.copyInto(vc3);
    L1Direction.update(unifL1Direction, transform3(vc3));
    L1Direction.dirty();
    L2BaseDirection.copyInto(vc3);
    L2Direction.update(unifL2Direction, transform3(vc3));
    L2Direction.dirty();

    var viewProjectionMatrix = projectionMatrix*viewMatrix;
    buildings.forEach((building) => building.update(viewProjectionMatrix, viewMatrix));
    flippedBuildings.forEach((building) => building.update(viewProjectionMatrix, viewMatrix));

    context.render();
  });

  limiter.fpsStream.listen((fps) {
    print("Frame rate: $fps fps");
  });
}

class Building extends Drawable {
  Matrix4 modelMatrix;
  static Matrix4 tempMatrix4 = new Matrix4.zero();
  static Matrix3 tempMatrix3 = new Matrix3.zero();

  sf.SetUniformCommand setMVPMatrix, setNormalMatrix;

  bool flipped;
  Drawable drawable;

  Building(int shape, this.drawable, num x, num y, num rot, {this.flipped: false}) {
    double buildingScale = superShapeParams[shape][SuperShapeParamCount-1];

    modelMatrix = new Matrix4.identity()
      ..translate(x, y, 0.0)
      ..rotateZ(rot)
      ..scale(buildingScale, buildingScale, buildingScale*(flipped ? -1 : 1));

    setMVPMatrix = new sf.SetUniformCommand(unifMVP, modelMatrix);
    setNormalMatrix = new sf.SetUniformCommand(unifNormalMatrix, modelMatrix);
  }

  void update(Matrix4 viewProjectionMatrix, Matrix4 viewMatrix) {
    viewProjectionMatrix.copyInto(tempMatrix4).multiply(modelMatrix);
    setMVPMatrix..update(unifMVP, tempMatrix4)..dirty();
    viewMatrix.copyInto(tempMatrix4).multiply(modelMatrix);
    setNormalMatrix..update(unifNormalMatrix, tempMatrix3..copyNormalMatrix(tempMatrix4))..dirty();
  }

  @override
  List<sf.Command> get commands => [setMVPMatrix, setNormalMatrix]..addAll(drawable.commands);
}
