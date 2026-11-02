#pragma once

#include <bgfx/bgfx.h>
namespace obx {

class Mesh {
  protected:
    bgfx::IndexBufferHandle indexBuffer;
    bgfx::VertexBufferHandle vertexBuffer;
    bgfx::VertexLayout layout;

    friend class Renderer;

  public:
    virtual void upload() = 0;
};
} // namespace obx
