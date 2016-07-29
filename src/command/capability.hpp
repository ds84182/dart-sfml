#pragma once

#include "../render_thread.hpp"

class CapabilityCommandData {
public:
  bool enable;
  GLenum cap;

  CapabilityCommandData() {}
};

using CapabilityCommandElement = RenderCommandElement<CapabilityCommandData, class CapabilityCommand>;

class CapabilityCommand : public RenderCommand<CapabilityCommandData> {
public:
  using RenderCommand::RenderCommand;

  virtual void doRender(bool justUpdated) {
    if (data.enable) {
      glEnable(data.cap);
    } else {
      glDisable(data.cap);
    }
  }
};
