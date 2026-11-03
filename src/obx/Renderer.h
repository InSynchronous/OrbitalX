#pragma once

#include "ImGuiRenderer.h"
#include "Mesh.h"
#include "Vertex.h"
#include "bx/math.h"
#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

namespace obx {
struct GLFWInfo {
    GLFWwindow *window;
    void *nativeWindow;
    void *nativeDisplay;
    bgfx::NativeWindowHandleType::Enum platformType;
};

struct Camera {
    bx::Vec3 position;
    bx::Vec3 rotation;
};

class Renderer {
  private:
    int width, height;
    GLFWInfo info;

    bgfx::IndexBufferHandle indexBuffer;
    bgfx::VertexBufferHandle vertexBuffer;
    bgfx::ProgramHandle program;
    bgfx::TextureHandle atlas;
    bgfx::UniformHandle sampler;

    std::vector<std::unique_ptr<Mesh>> meshes;
    ImGuiRenderer imguiRenderer;

    bgfx::ShaderHandle loadShader(const char *path);

    void setupGLFW();
    void setupImGui();
    void setupBGFX();
    void renderUI();
    void buildPrograms(std::string shaderDirectory);

  public:
    Renderer(int width, int height) : width(width), height(height) {
        this->initialize();
    }
    bool windowShouldClose() { return glfwWindowShouldClose(info.window); }
    void initialize();
    void addMesh(std::unique_ptr<Mesh> mesh);
    void loadTexture(const uint8_t *pixels, uint16_t w,
                     uint16_t h); // rgba 1 byte each (4.29 billion)
    void render();

    Camera camera{{0, 0, 0}, {0, 0, 0}};
};
} // namespace obx
