#pragma once

#include "../render_thread.hpp"

#include <SFML/Graphics.hpp>

class ClearCommandData {
public:
  sf::Color color;

  ClearCommandData() {}
  ClearCommandData(sf::Color color) : color(color) {}
};

using ClearCommandElement = RenderCommandElement<ClearCommandData, class ClearCommand>;

class ClearCommand : public RenderCommand<ClearCommandData> {
public:
  using RenderCommand::RenderCommand;

  virtual void doRender(bool justUpdated) {
    //glClearColor(data.)
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
};
