#pragma once

#include "Entity.h"
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

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#else
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
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
    int resetWidth = 0, resetHeight = 0;
    GLFWInfo info;

    bgfx::IndexBufferHandle indexBuffer;
    bgfx::VertexBufferHandle vertexBuffer;
    bgfx::ProgramHandle program;
    bgfx::UniformHandle sampler;

    std::vector<std::shared_ptr<Entity>> entities;
    ImGuiRenderer imguiRenderer;

    bgfx::ShaderHandle loadShader(const char *path);

    void setupGLFW();
    void setupImGui();
    void setupBGFX();
    void renderUI();
    void buildPrograms(std::string shaderDirectory);

    void (*imgui_user_callback)();

  public:
    Renderer(int width, int height) : width(width), height(height) { this->initialize(); }
    bool windowShouldClose() { return glfwWindowShouldClose(info.window); }
    void initialize();
    void addEntity(std::shared_ptr<Entity> entity);
    void addImguiCallback(void (*imgui_user_callback)());
    void render();

    Camera camera{{0, 0, 0}, {0, 0, 0}};
};
} // namespace obx
