#pragma once

#include "../render_thread.hpp"
#include "../shader.hpp"

#include <SFML/Graphics.hpp>

class BindVertexArrayCommandData {
public:
  std::shared_ptr<VertexArray> vao;

  BindVertexArrayCommandData() {}
};

using BindVertexArrayCommandElement = RenderCommandElement<BindVertexArrayCommandData, class BindVertexArrayCommand>;

class BindVertexArrayCommand : public RenderCommand<BindVertexArrayCommandData> {
public:
  using RenderCommand::RenderCommand;

  virtual void doRender(RenderThread *thread, bool justUpdated) {
    if (data.vao) {
      data.vao->bind();
    }
  }
};
