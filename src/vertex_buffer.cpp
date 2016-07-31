#include "vertex_buffer.hpp"

VertexBuffer::~VertexBuffer() {
  if (id > 0) {
    glDeleteBuffers(1, &id);
  }
}

void VertexBuffer::init(void *data, size_t size, GLenum usage) {
  glGenBuffers(1, &id);
  glBindBuffer(GL_ARRAY_BUFFER, id);
  glBufferData(GL_ARRAY_BUFFER, size, data, usage);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
