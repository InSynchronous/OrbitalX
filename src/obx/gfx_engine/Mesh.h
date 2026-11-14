#pragma once

#include "bx/math.h"
#include <bgfx/bgfx.h>
namespace obx::gfx_engine {

enum class MeshType { TRIANGLE_MESH, LINE_MESH };

class Mesh {
  protected:
    bgfx::IndexBufferHandle indexBuffer;
    bgfx::VertexBufferHandle vertexBuffer;
    bgfx::VertexLayout layout;

    bx::Vec3 position;
    bx::Vec3 rotation;
    bx::Vec3 scale = {1.0f, 1.0f, 1.0f};

    MeshType meshType = MeshType::TRIANGLE_MESH;
    float lineColor[4] = {0.0f, 1.0f, 1.0f, 0.0f};

    friend class Renderer;

  public:
    Mesh(bx::Vec3 position, bx::Vec3 rotation) : position(position), rotation(rotation) {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    }
    void setPosition(bx::Vec3 p) { position = p; }
    void setRotation(bx::Vec3 r) { rotation = r; }

    void setScale(bx::Vec3 v) { scale = v; }

    virtual void upload() = 0;
};
} // namespace obx::gfx_engine
