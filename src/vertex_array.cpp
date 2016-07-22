#include "vertex_array.hpp"

#include <cstdio>

VertexArray::~VertexArray() {
  if (id > 0) {
    glDeleteVertexArrays(1, &id);
  }
}

void VertexArray::init() {
  printf("VAO init\n");
  glGenVertexArrays(1, &id);
}
