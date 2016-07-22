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

  void bindAttributeLocation(const std::string &name, const Attribute attrib) {
    glBindAttribLocation(program, attrib, name.c_str());
  }

  Attribute getAttributeLocation(const std::string &name) const {
    return glGetAttribLocation(program, name.c_str());
  }

  Uniform getUniformLocation(const std::string &name) const {
    return glGetUniformLocation(program, name.c_str());
  }

  void bind() const {
    glUseProgram(program);
  }
};
