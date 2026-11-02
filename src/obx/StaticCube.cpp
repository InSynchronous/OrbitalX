#include "StaticCube.h"
#include <cassert>
#include <iostream>

void obx::StaticCube::upload() {

    for (auto &v : vertices) {
        v *= scale;
    }

    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .end();

    vertexBuffer = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices, sizeof(vertices)), layout);

    indexBuffer =
        bgfx::createIndexBuffer(bgfx::makeRef(indices, sizeof(indices)));

    assert(bgfx::isValid(indexBuffer));
    assert(bgfx::isValid(vertexBuffer));
}
