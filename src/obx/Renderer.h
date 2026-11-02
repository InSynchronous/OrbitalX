#pragma once

#include "Mesh.h"
#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
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

struct Vertex {
    float x, y, z;
    float r, g, b, a;

    Vertex &operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        r *= scalar;
        g *= scalar;
        b *= scalar;
        a *= scalar;
        return *this;
    }
};

class Renderer {
  private:
    int width, height;
    GLFWInfo info;

    bgfx::IndexBufferHandle indexBuffer;
    bgfx::VertexBufferHandle vertexBuffer;
    bgfx::ProgramHandle program;

    std::vector<std::unique_ptr<Mesh>> meshes;

    bgfx::ShaderHandle loadShader(const char *path);

    void setupGLFW();
    void setupBGFX();
    void buildPrograms(std::string shaderDirectory);

  public:
    Renderer(int width, int height) : width(width), height(height) {
        this->initialize();
    }
    bool windowShouldClose() { return glfwWindowShouldClose(info.window); }
    void initialize();
    void addMesh(std::unique_ptr<Mesh> mesh);
    void render();
};
} // namespace obx
