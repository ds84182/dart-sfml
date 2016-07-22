#include "shader.hpp"

bool Shader::compile(std::string vs, std::string fs, std::unordered_map<GLuint, std::string> attrLoc) {
  program = glCreateProgram();
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  const char *vsCS = vs.c_str();
  glShaderSource(vertexShader, 1, &vsCS, NULL);
  glCompileShader(vertexShader);

  GLint shaderCompiled = GL_FALSE;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);
  if (shaderCompiled != GL_TRUE) {
    printf("Unable to compile vertex shader %d!\n", vertexShader);

    errorLog = std::string(1024, '\0');
    GLsizei newLen = 0;
    glGetShaderInfoLog(vertexShader, errorLog.capacity(), &newLen, &errorLog[0]);
    errorLog.resize(newLen);

    cleanUp();

    return false;
  }

  glAttachShader(program, vertexShader);

  //////////

  const char *fsCS = fs.c_str();
  glShaderSource(fragmentShader, 1, &fsCS, NULL);
  glCompileShader(fragmentShader);

  shaderCompiled = GL_FALSE;
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);
  if (shaderCompiled != GL_TRUE) {
    printf("Unable to compile fragment shader %d!\n", fragmentShader);

    errorLog = std::string(1024, '\0');
    GLsizei newLen = 0;
    glGetShaderInfoLog(fragmentShader, errorLog.capacity(), &newLen, &errorLog[0]);
    errorLog.resize(newLen);

    cleanUp();

    return false;
  }

  glAttachShader(program, fragmentShader);

  //////////

  for (const auto &attrPair : attrLoc) {
    glBindAttribLocation(program, attrPair.first, attrPair.second.c_str());
  }

  glLinkProgram(program);

  GLint programSuccess = GL_TRUE;
  glGetProgramiv(program, GL_LINK_STATUS, &programSuccess);
  if (programSuccess != GL_TRUE) {
    printf("Error linking program %d!\n", program);

    errorLog = std::string(1024, '\0');
    GLsizei newLen = 0;
    glGetProgramInfoLog(program, errorLog.capacity(), &newLen, &errorLog[0]);
    errorLog.resize(newLen);

    cleanUp();

    return false;
  }

  return true;
}
