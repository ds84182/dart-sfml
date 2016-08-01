part of san_angeles;

const int SuperShapeParamCount = 15;

const List<List<num>> superShapeParams = const [
  //       m  a     b     n1      n2     n3     m     a     b     n1     n2      n3   res1 res2 scale  (org.res1,res2)
  const [ 10, 1,    2,    90,      1,   -45,    8,    1,    1,    -1,     1,   -0.4,   20,  30, 2 ], // 40, 60
  const [ 10, 1,    2,    90,      1,   -45,    4,    1,    1,    10,     1,   -0.4,   20,  20, 4 ], // 40, 40
  const [ 10, 1,    2,    60,      1,   -10,    4,    1,    1,    -1,    -2,   -0.4,   41,  41, 1 ], // 82, 82
  const [  6, 1,    1,    60,      1,   -70,    8,    1,    1,   0.4,     3,   0.25,   20,  20, 1 ], // 40, 40
  const [  4, 1,    1,    30,      1,    20,   12,    1,    1,   0.4,     3,   0.25,   10,  30, 1 ], // 20, 60
  const [  8, 1,    1,    30,      1,    -4,    8,    2,    1,    -1,     5,    0.5,   25,  26, 1 ], // 60, 60
  const [ 13, 1,    1,    30,      1,    -4,   13,    1,    1,     1,     5,      1,   30,  30, 6 ], // 60, 60
  const [ 10, 1,  1.1,  -0.5,    0.1,    70,   60,    1,    1,   -90,     0,  -0.25,   20,  60, 8 ], // 60, 180
  const [  7, 1,    1,    20,   -0.3,  -3.5,    6,    1,    1,    -1,   4.5,    0.5,   10,  20, 4 ], // 60, 80
  const [  4, 1,    1,    10,     10,    10,    4,    1,    1,    10,    10,     10,   10,  20, 1 ], // 20, 40
  const [  4, 1,    1,     1,      1,     1,    4,    1,    1,     1,     1,      1,   10,  10, 2 ], // 10, 10
  const [  1, 1,    1,    38,  -0.25,    19,    4,    1,    1,    10,    10,     10,   10,  15, 2 ], // 20, 40
  const [  2, 1,    1,   0.7,    0.3,   0.2,    3,    1,    1,   100,   100,    100,   10,  25, 2 ], // 20, 50
  const [  6, 1,    1,     1,      1,     1,    3,    1,    1,     1,     1,      1,   30,  30, 2 ], // 60, 60
  const [  3, 1,    1,     1,      1,     1,    6,    1,    1,     2,     1,      1,   10,  20, 2 ], // 20, 40
  const [  6, 1,    1,     6,    5.5,   100,    6,    1,    1,    25,    10,     10,   30,  20, 2 ], // 60, 40
  const [  3, 1,    1,   0.5,    1.7,   1.7,    2,    1,    1,    10,    10,     10,   20,  20, 2 ], // 40, 40
  const [  5, 1,    1,   0.1,    1.7,   1.7,    1,    1,    1,   0.3,   0.5,    0.5,   20,  20, 4 ], // 40, 40
  const [  2, 1,    1,     6,    5.5,   100,    6,    1,    1,     4,    10,     10,   10,  22, 1 ], // 40, 40
  const [  6, 1,    1,    -1,     70,   0.1,    9,    1,  0.5,   -98,  0.05,    -45,   20,  30, 4 ], // 60, 91
  const [  6, 1,    1,    -1,     90,  -0.1,    7,    1,    1,    90,   1.3,     34,   13,  16, 1 ], // 32, 60
];

void superShapeMap(Vector3 point, double r1, double r2, double t, double p) {
    // sphere-mapping of supershape parameters
    point.setValues(
      math.cos(t) * math.cos(p) / r1 / r2,
      math.sin(t) * math.cos(p) / r1 / r2,
      math.sin(p) / r2
    );
}

double ssFunc(double t, List<double> p, [int offset = 0]) {
  return math.pow(math.pow((math.cos(p[0+offset] * t / 4).abs()) / p[1+offset], p[4+offset]) +
                  math.pow((math.sin(p[0+offset] * t / 4)).abs() / p[2+offset], p[5+offset]), 1 / p[3+offset]);
}

/*
 * Creates and returns a supershape object.
 * Based on Paul Bourke's POV-Ray implementation.
 * http://astronomy.swin.edu.au/~pbourke/povray/supershape/
 */
DisplayObject createSuperShape(List<num> params) {
  int resol1 = params[SuperShapeParamCount-3].toInt();
  int resol2 = params[SuperShapeParamCount-2].toInt();

  // latitude 0 to pi/2 for no mirrored bottom
  // (latitudeBegin==0 for -pi/2 to pi/2 originally)

  int latitudeBegin = resol2 ~/ 4;
  int latitudeEnd = resol2 ~/ 2; // non-inclusive
  int longitudeCount = resol1;
  int latitudeCount = latitudeEnd-latitudeBegin;
  int triangleCount = longitudeCount * latitudeCount * 2;
  int vertices = triangleCount * 3;

  DisplayObject result = new DisplayObject(vertices, 3, true);

  List<double> baseColor = [
    ((randomUInt() % 155) + 100) / 255.0,
    ((randomUInt() % 155) + 100) / 255.0,
    ((randomUInt() % 155) + 100) / 255.0,
  ];

  int currentVertex = 0;

  // longitude -pi to pi
  for (int longitude = 0; longitude < longitudeCount; longitude++) {
    // latitude 0 to pi/2
    for (int latitude = latitudeBegin; latitude < latitudeEnd; latitude++) {
      double t1 = -math.PI + longitude * 2.0 * math.PI / resol1;
      double t2 = -math.PI + (longitude + 1) * 2.0 * math.PI / resol1;
      double p1 = -math.PI / 2.0 + latitude * 2.0 * math.PI / resol2;
      double p2 = -math.PI / 2.0 + (latitude + 1) * 2.0 * math.PI / resol2;

      double r0, r1, r2, r3;
      r0 = ssFunc(t1, params);
      r1 = ssFunc(p1, params, 6);
      r2 = ssFunc(t2, params);
      r3 = ssFunc(p2, params, 6);

      if (r0 != 0.0 && r1 != 0.0 && r2 != 0.0 && r3 != 0.0) {
        Vector3 pa = new Vector3.zero(), pb = new Vector3.zero(), pc = new Vector3.zero(), pd = new Vector3.zero();
        Vector3 v1, v2, n = new Vector3.zero();

        superShapeMap(pa, r0, r1, t1, p1);
        superShapeMap(pb, r2, r1, t2, p1);
        superShapeMap(pc, r2, r3, t2, p2);
        superShapeMap(pd, r0, r3, t1, p2);

        // kludge to set lower edge of the object to fixed level
        if (latitude == latitudeBegin + 1) {
          pa.z = pb.z = 0.0;
        }

        v1 = pb-pa;
        v2 = pd-pa;

        // Calculate normal with cross product.
        /*   i    j    k      i    j
         * v1.x v1.y v1.z | v1.x v1.y
         * v2.x v2.y v2.z | v2.x v2.y
         */
        n.x = v1.y * v2.z - v1.z * v2.y;
        n.y = v1.z * v2.x - v1.x * v2.z;
        n.z = v1.x * v2.y - v1.y * v2.x;

        double ca = pa.z + 0.5;

        for (int i = currentVertex * 3; i < (currentVertex + 6) * 3; i += 3) {
          n.copyIntoArray(result.normalArray, i);
        }

        List<int> color = baseColor.map((c) => math.max(0, math.min(ca*c*255, 255)).toInt().toUnsigned(8)).toList(growable: false);
        int colorInt = ((color[2]) << 16) |
          ((color[1]) << 8) | (color[0]);
        for (int i = currentVertex; i < currentVertex + 6; i++) {
          result.colorArray[i] = colorInt|0xFF000000;
        }

        pa.copyIntoArray(result.vertexArray, currentVertex++ * 3);
        pb.copyIntoArray(result.vertexArray, currentVertex++ * 3);
        pd.copyIntoArray(result.vertexArray, currentVertex++ * 3);
        pb.copyIntoArray(result.vertexArray, currentVertex++ * 3);
        pc.copyIntoArray(result.vertexArray, currentVertex++ * 3);
        pd.copyIntoArray(result.vertexArray, currentVertex++ * 3);
      }
    }
  }

  // Set number of vertices in object to the actual amount created.
  result.count = currentVertex;
  result.finalize();

  return result;
}
