#pragma once

#include "../render_thread.hpp"
#include "../shader.hpp"

#include <SFML/Graphics.hpp>

class DrawArraysCommandData {
public:
  GLenum mode = 0;
  GLint first;
  GLsizei count;

  DrawArraysCommandData() {}
};

using DrawArraysCommandElement = RenderCommandElement<DrawArraysCommandData, class DrawArraysCommand>;

class DrawArraysCommand : public RenderCommand<DrawArraysCommandData> {
public:
  using RenderCommand::RenderCommand;

  virtual void doRender(sf::RenderTarget *target, bool justUpdated) {
    if (data.mode) {
      glDrawArrays(data.mode, data.first, data.count);
    }
  }
};
