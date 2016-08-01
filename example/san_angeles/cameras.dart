part of san_angeles;

/*
 * Length in milliseconds of one camera track base unit.
 * The value originates from the music synchronization.
 */
const int CamTrackLength = 5442;

/// Camera track definition for one camera trucking shot.
class CamTrack {
  /// Camera parameters to tween between
  final CamParams src, dest;
  final int dist; // If >0, cam rotates around eye xy on dist * 0.1
  final int len; // Length multiplier

  const CamTrack(this.src, this.dest, this.dist, this.len);
}

class CamParams {
  final num eyeX, eyeY, eyeZ, viewAngle, viewHeightOffset;

  const CamParams(this.eyeX, this.eyeY, this.eyeZ, this.viewAngle, this.viewHeightOffset);

  CamParams lerp(CamParams other, double trackPos) {
    num l(num a, num b) => (a+b*trackPos)*0.01;
    return new CamParams(
      l(eyeX, other.eyeX),
      l(eyeY, other.eyeY),
      l(eyeZ, other.eyeZ),
      l(viewAngle, other.viewAngle),
      l(viewHeightOffset, other.viewHeightOffset)
    );
  }
}

const List<CamTrack> camTracks = const [
  const CamTrack(const CamParams(4500, 2700, 100, 70, -30), const CamParams(50, 50, -90, -100, 0), 20, 1),
  const CamTrack(const CamParams(-1448, 4294, 25, 363, 0), const CamParams(-136, 202, 125, -98, 100), 0, 1),
  const CamTrack(const CamParams(1437, 4930, 200, -275, -20), const CamParams(1684, 0, 0, 9, 0), 0, 1),
  const CamTrack(const CamParams(1800, 3609, 200, 0, 675), const CamParams(0, 0, 0, 300, 0), 0, 1),
  const CamTrack(const CamParams(923, 996, 50, 2336, -80), const CamParams(0, -20, -50, 0, 170), 0, 1),
  const CamTrack(const CamParams(-1663, -43, 600, 2170, 0), const CamParams(20, 0, -600, 0, 100), 0, 1),
  const CamTrack(const CamParams(1049, -1420, 175, 2111, -17), const CamParams(0, 0, 0, -334, 0), 0, 2),
  const CamTrack(const CamParams(0, 0, 50, 300, 25), const CamParams(0, 0, 0, 300, 0), 70, 2),
  const CamTrack(const CamParams(-473, -953, 3500, -353, -350), const CamParams(0, 0, -2800, 0, 0), 0, 2),
  const CamTrack(const CamParams(191, 1938, 35, 1139, -17), const CamParams(1205, -2909, 0, 0, 0), 0, 2),
  const CamTrack(const CamParams(-1449, -2700, 150, 0, 0), const CamParams(0, 2000, 0, 0, 0), 0, 2),
  const CamTrack(const CamParams(5273, 4992, 650, 373, -50), const CamParams(-4598, -3072, 0, 0, 0), 0, 2),
  const CamTrack(const CamParams(3223, -3282, 1075, -393, -25), const CamParams(1649, -1649, 0, 0, 0), 0, 2)
];

int currentCamTrack = 0;
int currentCamTrackStartTick = 0;
int nextCamTrackStartTick = 0x7fffffff;

final UP = new Vector3(0.0,0.0,1.0);

void camTrack(Matrix4 out) {
  double eX, eY, eZ, cX, cY, cZ;
  double trackPos;
  CamTrack cam;
  int currentCamTick;

  if (nextCamTrackStartTick <= tick) {
    currentCamTrack++;
    if (currentCamTrack >= camTracks.length) currentCamTrack = 0;
    currentCamTrackStartTick = nextCamTrackStartTick;
  }
  cam = camTracks[currentCamTrack];
  nextCamTrackStartTick = currentCamTrackStartTick+(cam.len*CamTrackLength);
  currentCamTick = tick - currentCamTrackStartTick;
  trackPos = currentCamTick / (CamTrackLength*cam.len);

  CamParams lerp = cam.src.lerp(cam.dest, trackPos);

  if (cam.dist > 0) {
    double dist = cam.dist*0.1;
    cX = lerp.eyeX;
    cY = lerp.eyeY;
    cZ = lerp.eyeZ;
    eX = cX - math.cos(lerp.viewAngle)*dist;
    eY = cY - math.sin(lerp.viewAngle)*dist;
    eZ = cZ - lerp.viewHeightOffset;
  } else {
    eX = lerp.eyeX;
    eY = lerp.eyeY;
    eZ = lerp.eyeZ;
    cX = eX + math.cos(lerp.viewAngle);
    cY = eY + math.sin(lerp.viewAngle);
    cZ = eZ + lerp.viewHeightOffset;
  }

  setViewMatrix(out, new Vector3(eX, eY, eZ), new Vector3(cX, cY, cZ), UP);
}
