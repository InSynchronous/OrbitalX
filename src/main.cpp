#include <iostream>

#include <GLFW/glfw3.h>

// I do not have any intention to support nonlinux builds (yet)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

// cpp
#include <fstream>
#include <iterator>
#include <vector>

// bgfx
#include <bgfx/bgfx.h>
#include <bx/math.h>

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
};

// ==start stolen from some random tutorial==
static const Vertex vertices[] = {
    {-1, -1, -1, 1, 0, 0, 1}, {1, -1, -1, 0, 1, 0, 1},
    {1, 1, -1, 0, 0, 1, 1},   {-1, 1, -1, 1, 1, 0, 1},

    {-1, -1, 1, 1, 0, 1, 1},  {1, -1, 1, 0, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1},    {-1, 1, 1, 1, 1, 1, 1},
};

static const uint16_t indices[] = {
    4, 5, 6, 4, 6, 7, // Front
    0, 2, 1, 0, 3, 2, // Back
    3, 7, 6, 3, 6, 2, // Top
    0, 1, 5, 0, 5, 4, // Bottom
    1, 2, 6, 1, 6, 5, // Right
    0, 4, 7, 0, 7, 3  // Left
};
// ==end stolen from a random tutorial==

bgfx::IndexBufferHandle indexBuffer;
bgfx::VertexBufferHandle vertexBuffer;

GLFWInfo setup_glfw() {
    if (!glfwInit()) {
        std::cout << "glfw init failed -1";
        std::exit(-1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window =
        glfwCreateWindow(1280, 720, "orbitalX", nullptr, nullptr);

    if (!window) {
        std::cout << "glfw window failed to open -1";
        glfwTerminate();
        std::exit(-1);
    }

    // windowing
    void *nativeWindow = nullptr;
    void *nativeDisplay = nullptr;

    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
        nativeWindow = glfwGetWaylandWindow(window);
        nativeDisplay = glfwGetWaylandDisplay();
    } else if (glfwGetPlatform() == GLFW_PLATFORM_X11) {
        nativeWindow = reinterpret_cast<void *>(
            static_cast<uintptr_t>(glfwGetX11Window(window)));

        nativeDisplay = glfwGetX11Display();
    } else {
        std::cout << "unsupported platform. use linux bum" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bgfx::NativeWindowHandleType::Enum platformType =
        bgfx::NativeWindowHandleType::Default;

    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
        nativeWindow = glfwGetWaylandWindow(window);
        nativeDisplay = glfwGetWaylandDisplay();
        platformType = bgfx::NativeWindowHandleType::Wayland;
    } else if (glfwGetPlatform() == GLFW_PLATFORM_X11) {
        nativeWindow = reinterpret_cast<void *>(
            static_cast<uintptr_t>(glfwGetX11Window(window)));

        nativeDisplay = glfwGetX11Display();
    } else {
        std::cerr << "Unsupported platform\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        std::exit(-1);
    }

    return {window, nativeWindow, nativeDisplay, platformType};

    return {window, nativeWindow, nativeDisplay, platformType};
}

void init_bgfx(GLFWInfo info) {
    bgfx::Init init;

    init.type = bgfx::RendererType::Count;

    init.platformData.type = info.platformType;
    init.platformData.context = nullptr;
    init.platformData.queue = nullptr;

    init.swapChain.nwh = info.nativeWindow;
    init.swapChain.ndt = info.nativeDisplay;
    init.swapChain.width = 1280;
    init.swapChain.height = 720;

    init.reset = BGFX_RESET_VSYNC;

    if (!bgfx::init(init)) {
        std::cerr << "Failed to initialize bgfx" << std::endl;

        glfwDestroyWindow(info.window);
        glfwTerminate();
        std::exit(-1);
    }

    bgfx::VertexLayout layout;
    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .end();

    // upload (vbo)
    vertexBuffer = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices, sizeof(vertices)), layout);

    indexBuffer =
        bgfx::createIndexBuffer(bgfx::makeRef(indices, sizeof(indices)));
}

bgfx::ShaderHandle loadShader(const char *path) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Failed to open shader: " << path << "\n";
        return BGFX_INVALID_HANDLE;
    }

    std::vector<char> data{std::istreambuf_iterator<char>(file),
                           std::istreambuf_iterator<char>()};

    const bgfx::Memory *memory =
        bgfx::copy(data.data(), static_cast<uint32_t>(data.size()));
    return bgfx::createShader(memory);
}

bgfx::ProgramHandle buildPrograms(std::string shaderDirectory, GLFWInfo info) {
    std::string vertexPath = std::string(shaderDirectory) + "vs_cube.bin";
    std::string fragmentPath = std::string(shaderDirectory) + "fs_cube.bin";

    bgfx::ShaderHandle vertexShader = loadShader(vertexPath.c_str());
    bgfx::ShaderHandle fragmentShader = loadShader(fragmentPath.c_str());

    if (!bgfx::isValid(vertexShader) || !bgfx::isValid(fragmentShader)) {
        std::cerr << "Shader loading failed\n";
        bgfx::shutdown();
        glfwDestroyWindow(info.window);
        glfwTerminate();
        std::exit(-1);
    }

    bgfx::ProgramHandle program =
        bgfx::createProgram(vertexShader, fragmentShader, true);

    return program;
}

void render(GLFWInfo glfwInfo, bgfx::ProgramHandle program) {
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(glfwInfo.window, &width, &height);

    if (width == 0 || height == 0)
        return;

    bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(width),
                      static_cast<uint16_t>(height));

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x202838ff, 1.0f,
                       0);

    float model[16];
    float view[16];
    float projection[16];

    float time = static_cast<float>(glfwGetTime());

    bx::mtxRotateXY(model, time, time * 0.7f);

    const bx::Vec3 eye = {0.0f, 0.0f, -5.0f};
    const bx::Vec3 at = {0.0f, 0.0f, 0.0f};
    const bx::Vec3 up = {0.0f, 1.0f, 0.0f};

    bx::mtxLookAt(view, eye, at, up);

    bx::mtxProj(projection, 60.0f, static_cast<float>(width) / height, 0.1f,
                100.0f, bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(0, view, projection);

    bgfx::setTransform(model);

    bgfx::setVertexBuffer(0, vertexBuffer);
    bgfx::setIndexBuffer(indexBuffer);

    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                   BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS);

    bgfx::submit(0, program);

    bgfx::frame();
}

} // namespace obx

int main() {
    std::cout << "Build system working?! " << std::endl;

    obx::GLFWInfo glfwInfo = obx::setup_glfw();
    obx::init_bgfx(glfwInfo);

    const char *shaderDirectory = nullptr;

    switch (bgfx::getRendererType()) {
    case bgfx::RendererType::Vulkan:
        shaderDirectory = "assets/shaders/spirv/";
        break;
    default:
        std::cerr << "Unsupported renderer\n";
        bgfx::shutdown();
        glfwTerminate();
        return -1;
    }

    bgfx::ProgramHandle program = obx::buildPrograms(shaderDirectory, glfwInfo);

    while (!glfwWindowShouldClose(glfwInfo.window)) {
        obx::render(glfwInfo, program);
    }

    return 0;
}
