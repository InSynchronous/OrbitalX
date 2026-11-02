#include "obx/Renderer.h"
#include "obx/StaticCube.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "Build system working?! " << std::endl;

    obx::Renderer renderer = obx::Renderer(1280, 720);
    obx::StaticCube cube = obx::StaticCube(0.8);

    renderer.addMesh(std::make_unique<obx::StaticCube>(cube));

    while (!renderer.windowShouldClose()) {
        renderer.render();
    }

    return 0;
}
