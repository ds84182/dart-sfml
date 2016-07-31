#include "index_buffer.hpp"

IndexBuffer::~IndexBuffer() {
  if (id > 0) {
    glDeleteBuffers(1, &id);
  }
}

void IndexBuffer::init(void *data, size_t size, GLenum type, GLenum usage) {
  glGenBuffers(1, &id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  this->type = type;
}
