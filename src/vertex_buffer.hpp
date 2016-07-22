#pragma once

#include "ogl.hpp"

class VertexBuffer {
public:
  VertexBuffer() {}
  ~VertexBuffer();

  void init(void *data, size_t size, GLenum usage);

  GLuint id = 0;
};
