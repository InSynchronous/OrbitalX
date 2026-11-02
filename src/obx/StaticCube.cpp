#include "StaticCube.h"
#include <cassert>

void obx::StaticCube::upload() {

    for (auto &v : vertices) {
        v.position = bx::mul(v.position, scale);
    }

    vertexBuffer = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices, sizeof(vertices)), layout);

    indexBuffer =
        bgfx::createIndexBuffer(bgfx::makeRef(indices, sizeof(indices)));

    assert(bgfx::isValid(indexBuffer));
    assert(bgfx::isValid(vertexBuffer));
}
