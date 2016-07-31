#pragma once

#include "../render_thread.hpp"

#include <string>
#include <vector>

enum class UniformType {
  Float, Float2, Float3, Float4,
  Int, Int2, Int3, Int4,
  Matrix2, Matrix3, Matrix4
};

class SetUniformCommandData {
public:
  std::string name;
  UniformType type;
  std::vector<char> data;
  int count;

  SetUniformCommandData() {}
};

using SetUniformCommandElement = RenderCommandElement<SetUniformCommandData, class SetUniformCommand>;

class SetUniformCommand : public RenderCommand<SetUniformCommandData> {
public:
  using RenderCommand::RenderCommand;

  virtual void doRender(RenderThread *thread, bool justUpdated) {
    if (justUpdated) {
      GLint location = thread->shader->getUniformLocation(data.name);
      auto count = data.count;
      SneekyPointer dataptr = data.data.data();
      switch (data.type) {
        case UniformType::Float:
          glUniform1fv(location, count, dataptr);
          break;
        case UniformType::Float2:
          glUniform2fv(location, count, dataptr);
          break;
        case UniformType::Float3:
          glUniform3fv(location, count, dataptr);
          break;
        case UniformType::Float4:
          glUniform4fv(location, count, dataptr);
          break;

        case UniformType::Int:
          glUniform1iv(location, count, dataptr);
          break;
        case UniformType::Int2:
          glUniform2iv(location, count, dataptr);
          break;
        case UniformType::Int3:
          glUniform3iv(location, count, dataptr);
          break;
        case UniformType::Int4:
          glUniform4iv(location, count, dataptr);
          break;

        case UniformType::Matrix2:
          glUniformMatrix2fv(location, count, false, dataptr);
          break;
        case UniformType::Matrix3:
          glUniformMatrix3fv(location, count, false, dataptr);
          break;
        case UniformType::Matrix4:
          glUniformMatrix4fv(location, count, false, dataptr);
          break;
      }
    }
  }
};
