#include "ConeMesh.h"
#include "Vertex.h"
#include "bx/math.h"

#include <bgfx/bgfx.h>
#include <cmath>
#include <cstdint>
#include <vector>

obx::gfx_engine::ConeMesh::ConeMesh(const bx::Vec3 &position, const bx::Vec3 &rotation,
                                    float height, float baseRadius, int segments,
                                    const bx::Vec3 &apex, const bx::Vec3 &axis)
    : Mesh(position, rotation), height(height), baseRadius(baseRadius), segments(segments),
      apex(apex), axis(bx::normalize(axis)) {}

void obx::gfx_engine::ConeMesh::upload() {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    constexpr float twoPi = 6.28318530718f;

    const bx::Vec3 helper =
        (std::fabs(axis.y) < 0.99f) ? bx::Vec3{0.0f, 1.0f, 0.0f} : bx::Vec3{1.0f, 0.0f, 0.0f};
    const bx::Vec3 u = bx::normalize(bx::cross(helper, axis));
    const bx::Vec3 v = bx::cross(axis, u);

    auto basePoint = [&](float a) {
        const float cx = baseRadius * std::cos(a);
        const float cz = baseRadius * std::sin(a);
        return bx::add(bx::add(apex, bx::mul(axis, height)),
                       bx::add(bx::mul(u, cx), bx::mul(v, cz)));
    };

    for (int i = 0; i < segments; ++i) {
        const float t0 = static_cast<float>(i) / segments;
        const float t1 = static_cast<float>(i + 1) / segments;

        const bx::Vec3 p0 = basePoint(twoPi * t0);
        const bx::Vec3 p1 = basePoint(twoPi * t1);

        const bx::Vec3 pts[3] = {apex, p1, p0};
        const float us[3] = {(t0 + t1) * 0.5f, t1, t0};
        const float vs[3] = {0.0f, 1.0f, 1.0f};

        for (int k = 0; k < 3; ++k) {
            indices.push_back(static_cast<uint16_t>(vertices.size()));
            vertices.push_back(Vertex(pts[k], {us[k], vs[k]}));
        }
    }

    vertexBuffer = bgfx::createVertexBuffer(
        bgfx::copy(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(Vertex))),
        layout);
    indexBuffer = bgfx::createIndexBuffer(
        bgfx::copy(indices.data(), static_cast<uint32_t>(indices.size() * sizeof(uint16_t))));
}

void obx::gfx_engine::ConeMesh::rebuild(float newHeight, float newBaseRadius,
                                        const bx::Vec3 &newApex, const bx::Vec3 &newAxis) {
    height = newHeight;
    baseRadius = newBaseRadius;
    apex = newApex;
    axis = bx::normalize(newAxis);

    if (bgfx::isValid(vertexBuffer))
        bgfx::destroy(vertexBuffer);
    if (bgfx::isValid(indexBuffer))
        bgfx::destroy(indexBuffer);

    upload();
}
