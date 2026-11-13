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

    std::vector<bx::Vec3> points;

  public:
    OrbitLineMesh(bx::Vec3 position, bx::Vec3 rotation, std::vector<bx::Vec3> &points)
        : Mesh{position, rotation}, points(points) {
        this->meshType = MeshType::LINE_MESH;
    }

    void upload() override;
    void rebuild(bx::Vec3 position, bx::Vec3 rotation, std::vector<bx::Vec3> &points);
};

} // namespace obx::gfx_engine
