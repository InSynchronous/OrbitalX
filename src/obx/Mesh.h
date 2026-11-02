#pragma once

#include "Vertex.h"
#include <bgfx/bgfx.h>
namespace obx {

class Mesh {
  protected:
    bgfx::IndexBufferHandle indexBuffer;
    bgfx::VertexBufferHandle vertexBuffer;
    bgfx::VertexLayout layout;

    Vertex position;

    friend class Renderer;

  public:
    Mesh(Vertex position) : position(position) {}
    virtual void upload() = 0;
};
} // namespace obx
