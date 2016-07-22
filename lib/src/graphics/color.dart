part of sfml;

class Color {
	final int raw;

	int get r => (raw >> 24) & 0xFF;
	int get g => (raw >> 16) & 0xFF;
	int get b => (raw >> 8) & 0xFF;
	int get a => (raw) & 0xFF;

	const Color.rgba(int r, int g, int b, [int a = 255]) : raw = ((r << 24)|(g << 16)|(b << 8)|a)&0xFFFFFFFF;
	const Color(this.raw);

	Color withR(int r) => new Color.rgba(r, g, b, a);
	Color withG(int g) => new Color.rgba(r, g, b, a);
	Color withB(int b) => new Color.rgba(r, g, b, a);
	Color withA(int a) => new Color.rgba(r, g, b, a);

	@override
	String toString() => "Color: $r, $g, $b, $a (${raw.toRadixString(16)})";

	@override
	int get hashCode => raw;
}
