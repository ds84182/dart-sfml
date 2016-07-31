#include "vertex_array.hpp"

VertexArray::~VertexArray() {
  if (id > 0) {
    glDeleteVertexArrays(1, &id);
  }
}

void VertexArray::init() {
  glGenVertexArrays(1, &id);
}
