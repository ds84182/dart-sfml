part of san_angeles;

int randomSeed = 0;

int randomUInt() {
  randomSeed = (randomSeed * 0x343fd + 0x269ec3).toUnsigned(32);
  return (randomSeed >> 16).toUnsigned(16);
}
