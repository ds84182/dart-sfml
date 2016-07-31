part of sfml;

enum RenderCapability {
  Blend,
  CullFace,
  DepthTest,
  PolygonOffset,
  ScissorTest,
  StencilTest
}

class EnableCapabilityCommand extends Command {
  EnableCapabilityCommand(RenderCapability capability) {
    _init(true);
    _set(capability.index);
  }

  void _init(bool enable) native "CapabilityCommand";

  void _set(int cap) native "CapabilityCommand::set";
}

class DisableCapabilityCommand extends Command {
  DisableCapabilityCommand(RenderCapability capability) {
    _init(false);
    _set(capability.index);
  }

  void _init(bool enable) native "CapabilityCommand";

  void _set(int cap) native "CapabilityCommand::set";
}
