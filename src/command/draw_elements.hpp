#pragma once

#include "../render_thread.hpp"
#include "../index_buffer.hpp"

class DrawElementsCommandData {
public:
  std::shared_ptr<IndexBuffer> ibo;
  GLenum mode;
  GLint first;
  GLsizei count;

  DrawElementsCommandData() {}
};

using DrawElementsCommandElement = RenderCommandElement<DrawElementsCommandData, class DrawElementsCommand>;

class DrawElementsCommand : public RenderCommand<DrawElementsCommandData> {
public:
  using RenderCommand::RenderCommand;

  virtual void doRender(RenderThread *thread, bool justUpdated) {
    if (data.ibo) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo->id);
      glDrawElements(data.mode, data.count, data.ibo->type, reinterpret_cast<void*>(data.first));
    }
  }
};
