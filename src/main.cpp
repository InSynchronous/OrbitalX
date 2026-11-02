#include "obx/ObjMesh.h"
#include "obx/Renderer.h"
#include "obx/SphereMesh.h"
#include "obx/StaticCube.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "Build system working?! " << std::endl;

    obx::Renderer renderer = obx::Renderer(1280, 720);
    obx::StaticCube cube = obx::StaticCube({-4, 0, 0}, 0.8);
    obx::SphereMesh sphere = obx::SphereMesh({0, 0, 3}, 4, 1);
    obx::ObjMesh teapot = obx::ObjMesh({3, 3, 3}, "assets/models/teapot.obj");

    renderer.addMesh(std::make_unique<obx::StaticCube>(cube));
    renderer.addMesh(std::make_unique<obx::SphereMesh>(sphere));
    renderer.addMesh(std::make_unique<obx::ObjMesh>(teapot));

    while (!renderer.windowShouldClose()) {
        renderer.render();
    }

    return 0;
}
