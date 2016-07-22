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

  virtual void doRender(sf::RenderTarget *target, bool justUpdated) {
    target->clear(data.color);
  }
};
