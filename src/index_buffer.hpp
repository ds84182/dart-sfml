#pragma once

#include "ogl.hpp"

class IndexBuffer {
public:
  IndexBuffer() {}
  ~IndexBuffer();

  void init(void *data, size_t size, GLenum type, GLenum usage);

  GLuint id = 0;
  GLenum type;
};
