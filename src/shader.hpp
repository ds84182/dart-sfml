#pragma once

#include "ogl.hpp"

#include "render_thread.hpp"

#include <string>
#include <unordered_map>

typedef GLuint Uniform;
typedef GLuint Attribute;

enum AttributeType: GLint {
  Float = GL_FLOAT
};

class Shader : public RenderResource {
public:
  GLuint program;
  GLuint vertexShader;
  GLuint fragmentShader;
  std::string errorLog;
  std::unordered_map<std::string, Uniform> uniforms;

  Shader() {};

  virtual ~Shader() {
    cleanUp();
  }

  bool compile(std::string vs, std::string fs, std::unordered_map<GLuint, std::string> attrLoc);

  void cleanUp() {
    if (program > 0) {
      glDeleteProgram(program);
      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);
      program = 0;
    }
  }

  Uniform getUniformLocation(const std::string &name) {
    if (uniforms.count(name)) {
      return uniforms[name];
    }
    return uniforms[name] = glGetUniformLocation(program, name.c_str());
  }

  void bind() const {
    glUseProgram(program);
  }
};
