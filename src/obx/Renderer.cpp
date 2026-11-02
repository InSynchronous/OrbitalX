#include "Renderer.h"
#include <iostream>

#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

#include <bx/math.h>

// ==start stolen from some random tutorial==
static const obx::Vertex vertices[] = {
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

void obx::Renderer::setupGLFW() {
    if (!glfwInit()) {
        std::cout << "glfw init failed -1";
        std::exit(-1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window =
        glfwCreateWindow(width, height, "orbitalX", nullptr, nullptr);

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

    this->info = {window, nativeWindow, nativeDisplay, platformType};
}

void obx::Renderer::setupBGFX() {
    bgfx::Init init;

    init.type = bgfx::RendererType::Count;

    init.platformData.type = info.platformType;
    init.platformData.context = nullptr;
    init.platformData.queue = nullptr;

    init.swapChain.nwh = info.nativeWindow;
    init.swapChain.ndt = info.nativeDisplay;
    init.swapChain.width = width;
    init.swapChain.height = height;

    init.reset = BGFX_RESET_VSYNC;

    if (!bgfx::init(init)) {
        std::cerr << "Failed to initialize bgfx" << std::endl;

        glfwDestroyWindow(info.window);
        glfwTerminate();
        std::exit(-1);
    }
}

bgfx::ShaderHandle obx::Renderer::loadShader(const char *path) {
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

void obx::Renderer::buildPrograms(std::string shaderDirectory) {
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

    program = bgfx::createProgram(vertexShader, fragmentShader, true);
}

void obx::Renderer::initialize() {
    this->setupGLFW();
    this->setupBGFX();
    const char *shaderDirectory = nullptr;

    switch (bgfx::getRendererType()) {
    case bgfx::RendererType::Vulkan:
        shaderDirectory = "assets/shaders/spirv/";
        break;
    default:
        std::cerr << "Unsupported renderer\n";
        bgfx::shutdown();
        glfwTerminate();
        std::exit(-1);
    }

    this->buildPrograms(shaderDirectory);
}

void obx::Renderer::addMesh(std::unique_ptr<Mesh> mesh) {
    mesh->upload();
    meshes.push_back(std::move(mesh));
}

void obx::Renderer::render() {
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(info.window, &width, &height);

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

    bx::mtxLookAt(view, {0.0f, 0.0f, -5.0f}, {0.0f, 0.0f, 0.0f},
                  {0.0f, 1.0f, 0.0f});

    bx::mtxProj(projection, 60.0f, static_cast<float>(width) / height, 0.1f,
                100.0f, bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(0, view, projection);

    for (auto &mesh : meshes) {
        bx::mtxSRT(model, 1.0f, 1.0f, 1.0f, time, time * 0.7f, 0.0f,
                   mesh->position.x, mesh->position.y, mesh->position.z);

        bgfx::setTransform(model);
        bgfx::setVertexBuffer(0, mesh->vertexBuffer);
        bgfx::setIndexBuffer(mesh->indexBuffer);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                       BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS);

        bgfx::submit(0, program);
    }

    bgfx::frame();
}
