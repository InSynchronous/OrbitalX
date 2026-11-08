#pragma once

#include "bgfx/bgfx.h"
#include "imgui.h"

class ImGuiRenderer {
  private:
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle sampler = BGFX_INVALID_HANDLE;
    bgfx::VertexLayout layout;
    bgfx::ViewId viewId = 255;

  public:
    void initialize();
    void render(ImDrawData *drawData);
    void shutdown();
};
