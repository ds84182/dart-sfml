part of san_angeles;

const sf.Attribute attrPosition = const sf.Attribute("attrPosition", 0);
const sf.Attribute attrColor = const sf.Attribute("attrColor", 1);
const sf.Attribute attrNormal = const sf.Attribute("attrNormal", 2);
const List<sf.Attribute> attributes = const [
  attrPosition, attrColor, attrNormal
];

const sf.Uniform unifMVP = const sf.Uniform("unifMVP", sf.UniformType.Matrix4);

const FLAT_VERTEX_SRC = """
attribute vec4 attrPosition;
attribute vec4 attrColor;
attribute vec3 attrNormal;

uniform mat4 unifProjection;
uniform mat4 unifView;
uniform mat4 unifModel;

varying vec4 varyColor;

void main() {
  varyColor = attrColor;
  gl_Position = unifProjection*unifView*(unifModel*attrPosition);
}
""";

const COMMON_FRAGMENT_SRC = """
varying vec4 varyColor;

void main() {
  gl_FragColor = varyColor;
}
""";

const sf.Uniform unifNormalMatrix = const sf.Uniform("unifNormalMatrix", sf.UniformType.Matrix3);

const sf.Uniform unifAmbient = const sf.Uniform("unifAmbient", sf.UniformType.Float4);
const sf.Uniform unifShininess = const sf.Uniform("unifShininess", sf.UniformType.Float);

const sf.Uniform unifL0Direction = const sf.Uniform("unifL0Direction", sf.UniformType.Float3);
const sf.Uniform unifL0Diffuse = const sf.Uniform("unifL0Diffuse", sf.UniformType.Float4);
const sf.Uniform unifL0Specular = const sf.Uniform("unifL0Specular", sf.UniformType.Float4);

const sf.Uniform unifL1Direction = const sf.Uniform("unifL1Direction", sf.UniformType.Float3);
const sf.Uniform unifL1Diffuse = const sf.Uniform("unifL1Diffuse", sf.UniformType.Float4);

const sf.Uniform unifL2Direction = const sf.Uniform("unifL2Direction", sf.UniformType.Float3);
const sf.Uniform unifL2Diffuse = const sf.Uniform("unifL2Diffuse", sf.UniformType.Float4);

const LIT_VERTEX_SRC = """
attribute vec4 attrPosition;
attribute vec4 attrColor;
attribute vec3 attrNormal;

uniform mat4 unifMVP;

uniform mat3 unifNormalMatrix;

uniform vec4 unifAmbient;
uniform float unifShininess;

uniform vec3 unifL0Direction;
uniform vec4 unifL0Diffuse;
uniform vec4 unifL0Specular;

uniform vec3 unifL1Direction;
uniform vec4 unifL1Diffuse;

uniform vec3 unifL2Direction;
uniform vec4 unifL2Diffuse;

varying vec4 varyColor;

vec3 worldNormal;

vec4 SpecularLight(vec3 direction, vec4 diffuseColor, vec4 specularColor) {
  vec3 lightDir = normalize(direction);
  float diffuse = max(0., dot(worldNormal, lightDir));
  float specular = 0.0;
  if (diffuse > 0.0) {
    vec3 halfv = normalize(lightDir + vec3(0., 0., 1.));
    specular = pow(max(0., dot(halfv, worldNormal)), unifShininess);
  }
  return diffuse * diffuseColor * attrColor + specular * specularColor;
}

vec4 DiffuseLight(vec3 direction, vec4 diffuseColor) {
  vec3 lightDir = normalize(direction);
  float diffuse = max(0., dot(worldNormal, lightDir));
  return diffuse * diffuseColor * attrColor;
}

void main() {
  worldNormal = normalize(unifNormalMatrix*attrNormal);

  gl_Position = unifMVP*attrPosition;

  vec4 color = unifAmbient * attrColor;
  color += SpecularLight(unifL0Direction, unifL0Diffuse, unifL0Specular);
  color += DiffuseLight(unifL1Direction, unifL1Diffuse);
  color += DiffuseLight(unifL2Direction, unifL2Diffuse);
  varyColor = color;
}
""";
