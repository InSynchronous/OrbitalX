#pragma once

#include "Mesh.h"
#include "Vertex.h"
#include "bx/math.h"
#include <cstdint>
#include <vector>

namespace obx::gfx_engine {

class OrbitLineMesh : public Mesh {
  private:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    std::shared_ptr<std::vector<bx::Vec3>> points;

  public:
    OrbitLineMesh(bx::Vec3 position, bx::Vec3 rotation,
                  std::shared_ptr<std::vector<bx::Vec3>> points)
        : Mesh{position, rotation}, points(points) {
        this->meshType = MeshType::LINE_MESH;
    }

    OrbitLineMesh(bx::Vec3 position, bx::Vec3 rotation,
                  std::shared_ptr<std::vector<bx::Vec3>> points, float r, float g, float b, float a)
        : Mesh{position, rotation}, points(points) {
        this->meshType = MeshType::LINE_MESH;
        this->lineColor[0] = r;
        this->lineColor[1] = g;
        this->lineColor[2] = b;
        this->lineColor[3] = a;
    }

    void upload() override;
    void rebuild(bx::Vec3 position, bx::Vec3 rotation,
                 std::shared_ptr<std::vector<bx::Vec3>> points);
};

} // namespace obx::gfx_engine
