#include "OrbitLineMesh.h"
#include "Vertex.h"

#include <bgfx/bgfx.h>

namespace obx::gfx_engine {
void OrbitLineMesh::upload() {
    // gpu upload
    vertices.clear();

    for (auto &point : points) {
        vertices.push_back(Vertex{point, {0, 0}});
    }

    vertexBuffer = bgfx::createVertexBuffer(
        bgfx::copy(vertices.data(), vertices.size() * sizeof(Vertex)), layout);

    // note: this is not a "MESH"; like it literally is just a list of points on the gpu to draw
    // line segments through
}

void OrbitLineMesh::rebuild(bx::Vec3 position, bx::Vec3 rotation, std::vector<bx::Vec3> &points) {
    this->position = position;
    this->rotation = rotation;
    this->points = points;

    if (bgfx::isValid(vertexBuffer))
        bgfx::destroy(vertexBuffer);

    this->upload();
}

} // namespace obx::gfx_engine
