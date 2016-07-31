#pragma once

#include "../render_thread.hpp"

#include <SFML/Graphics.hpp>

class ClearCommandData {
public:
  float color[4];
  GLbitfield mask;

  ClearCommandData() {}
};

using ClearCommandElement = RenderCommandElement<ClearCommandData, class ClearCommand>;

class ClearCommand : public RenderCommand<ClearCommandData> {
public:
  using RenderCommand::RenderCommand;

  virtual void doRender(RenderThread *thread, bool justUpdated) {
    if (justUpdated) {
      glClearColor(data.color[0], data.color[1], data.color[2], data.color[3]);
    }
    glClear(data.mask);
  }
};
