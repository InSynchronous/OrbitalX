#include "obx/ObjMesh.h"
#include "obx/Renderer.h"
#include "obx/SphereMesh.h"
#include "obx/StaticCube.h"
#include <iostream>
#include <memory>

std::vector<uint8_t> generate_checkerboard(int width, int height) {
    std::vector<uint8_t> pixels(width * height * 4);

    for (uint16_t y = 0; y < height; ++y) {
        for (uint16_t x = 0; x < width; ++x) {
            const bool checker = ((x / 32) + (y / 32)) % 2;

            const uint8_t color = checker ? 255 : 30;

            const size_t i = (y * width + x) * 4;

            pixels[i + 0] = color;
            pixels[i + 1] = color;
            pixels[i + 2] = color;
            pixels[i + 3] = 255;
        }
    }

    return pixels;
}

int main() {
    std::cout << "Build system working?! " << std::endl;

    obx::Renderer renderer = obx::Renderer(1280, 720);
    obx::StaticCube cube = obx::StaticCube({-4, 0, 0}, 0.8);
    obx::SphereMesh sphere = obx::SphereMesh({0, 0, 3}, 4, 1);
    obx::ObjMesh teapot = obx::ObjMesh({3, 3, 30}, "assets/models/teapot.obj");

    renderer.addMesh(std::make_unique<obx::StaticCube>(cube));
    renderer.addMesh(std::make_unique<obx::SphereMesh>(sphere));
    renderer.addMesh(std::make_unique<obx::ObjMesh>(teapot));

    std::vector<uint8_t> pixels = generate_checkerboard(1280, 720);
    renderer.loadTexture(pixels.data());

    while (!renderer.windowShouldClose()) {
        renderer.render();
    }

    return 0;
}
