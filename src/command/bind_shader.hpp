#pragma once

#include "../render_thread.hpp"
#include "../shader.hpp"

#include <SFML/Graphics.hpp>

class BindShaderCommandData {
public:
  std::shared_ptr<Shader> shader;

  BindShaderCommandData() {}
};

using BindShaderCommandElement = RenderCommandElement<BindShaderCommandData, class BindShaderCommand>;

class BindShaderCommand : public RenderCommand<BindShaderCommandData> {
public:
  using RenderCommand::RenderCommand;

  virtual void doRender(bool justUpdated) {
    if (data.shader) {
      data.shader->bind();
    }
  }
};
