#pragma once

#include "ogl.hpp"

class VertexArray {
public:
  VertexArray() {}
  ~VertexArray();

  void init();

  void bind() {
    glBindVertexArray(id);
  }

  GLuint id = 0;
};
