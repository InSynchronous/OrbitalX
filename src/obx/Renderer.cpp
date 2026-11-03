#include "Renderer.h"
#include "bgfx/bgfx.h"
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

#include <bx/math.h>

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

void obx::Renderer::setupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsClassic();
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui_ImplGlfw_InitForOther(info.window, true);
}

void obx::Renderer::initialize() {
    this->setupGLFW();
    this->setupBGFX();
    this->setupImGui();

    imguiRenderer.initialize();

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

    sampler = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
}

void obx::Renderer::addMesh(std::unique_ptr<Mesh> mesh) {
    mesh->upload();
    meshes.push_back(std::move(mesh));
}

void obx::Renderer::loadTexture(const uint8_t *pixels, uint16_t w, uint16_t h) {
    const bgfx::Memory *memory = bgfx::copy(pixels, w * h * 4);

    atlas = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::RGBA8,
                                  BGFX_SAMPLER_V_CLAMP, memory);
}

void obx::Renderer::renderUI() {
    ImGui::Begin("OrbitalX Debug");

    ImGui::Text("Performance");
    ImGui::Separator();

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

    ImGui::Spacing();
    ImGui::Text("Renderer");
    ImGui::Separator();

    ImGui::Text("Backend: %s", bgfx::getRendererName(bgfx::getRendererType()));

    ImGui::Text("Meshes: %zu", meshes.size());

    ImGui::Spacing();
    ImGui::Text("Input");
    ImGui::Separator();

    ImGui::Text("Mouse: %.0f, %.0f", ImGui::GetIO().MousePos.x,
                ImGui::GetIO().MousePos.y);

    ImGui::End();

    ImGui::Render();
    imguiRenderer.render(ImGui::GetDrawData());
}

void obx::Renderer::render() {
    glfwPollEvents();

    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int width, height;
    glfwGetFramebufferSize(info.window, &width, &height);

    if (width == 0 || height == 0)
        return;

    bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(width),
                      static_cast<uint16_t>(height));

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000, 1.0f,
                       0);

    float model[16];
    float view[16];
    float projection[16];

    float time = static_cast<float>(glfwGetTime());

    float cameraTransform[16];

    bx::mtxSRT(cameraTransform, 1.0f, 1.0f, 1.0f, camera.rotation.x,
               camera.rotation.y, camera.rotation.z, camera.position.x,
               camera.position.y, camera.position.z);

    bx::mtxInverse(view, cameraTransform);

    bx::mtxProj(projection, 60.0f, static_cast<float>(width) / height, 1.0f,
                20000.0f, bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(0, view, projection);

    for (auto &mesh : meshes) {
        bx::mtxSRT(model, 1.0f, 1.0f, 1.0f, mesh->rotation.x, mesh->rotation.y,
                   mesh->rotation.z, mesh->position.x, mesh->position.y,
                   mesh->position.z);

        bgfx::setTransform(model);
        bgfx::setVertexBuffer(0, mesh->vertexBuffer);
        bgfx::setIndexBuffer(mesh->indexBuffer);
        bgfx::setTexture(0, sampler, atlas);

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                       BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS);

        bgfx::submit(0, program);
    }

    this->renderUI();

    bgfx::frame();
}
