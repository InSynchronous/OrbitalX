#include "Renderer.h"
#include "Entity.h"
#include "bgfx/bgfx.h"
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

#include <bx/math.h>

using bx::mtxSRT;

void obx::gfx_engine::Renderer::setupGLFW() {
    if (!glfwInit()) {
        std::cout << "glfw init failed -1";
        std::exit(-1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window = glfwCreateWindow(width, height, "orbitalX", nullptr, nullptr);

    if (!window) {
        std::cout << "glfw window failed to open -1";
        glfwTerminate();
        std::exit(-1);
    }

    // windowing
#ifdef __APPLE__
    void *nativeWindow = glfwGetCocoaWindow(window);
    void *nativeDisplay = nullptr;
    bgfx::NativeWindowHandleType::Enum platformType = bgfx::NativeWindowHandleType::Default;
#else
    void *nativeWindow = nullptr;
    void *nativeDisplay = nullptr;
    bgfx::NativeWindowHandleType::Enum platformType = bgfx::NativeWindowHandleType::Default;
    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
        nativeWindow = glfwGetWaylandWindow(window);
        nativeDisplay = glfwGetWaylandDisplay();
        platformType = bgfx::NativeWindowHandleType::Wayland;
    } else if (glfwGetPlatform() == GLFW_PLATFORM_X11) {
        nativeWindow = reinterpret_cast<void *>(static_cast<uintptr_t>(glfwGetX11Window(window)));
        nativeDisplay = glfwGetX11Display();
    } else {
        std::cerr << "Unsupported platform\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        std::exit(-1);
    }
#endif

    this->info = {window, nativeWindow, nativeDisplay, platformType};
}

void obx::gfx_engine::Renderer::setupBGFX() {
    bgfx::Init init;
    init.type = bgfx::RendererType::Count;

    init.platformData.type = info.platformType;
    init.platformData.context = nullptr;
    init.platformData.queue = nullptr;

    init.swapChain.nwh = info.nativeWindow;
    init.swapChain.ndt = info.nativeDisplay;
    int fbw, fbh;
    glfwGetFramebufferSize(info.window, &fbw, &fbh);
    init.swapChain.width = fbw > 0 ? static_cast<uint32_t>(fbw) : width;
    init.swapChain.height = fbh > 0 ? static_cast<uint32_t>(fbh) : height;

    init.reset = BGFX_RESET_VSYNC;

    if (!bgfx::init(init)) {
        std::cerr << "Failed to initialize bgfx" << std::endl;

        glfwDestroyWindow(info.window);
        glfwTerminate();
        std::exit(-1);
    }
}

bgfx::ShaderHandle obx::gfx_engine::Renderer::loadShader(const char *path) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Failed to open shader: " << path << "\n";
        return BGFX_INVALID_HANDLE;
    }

    std::vector<char> data{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    const bgfx::Memory *memory = bgfx::copy(data.data(), static_cast<uint32_t>(data.size()));
    std::cout << "Loading shader: " << path << " (" << data.size() << " bytes)"
              << " Renderer: " << bgfx::getRendererName(bgfx::getRendererType()) << '\n';

    return bgfx::createShader(memory);
}

void obx::gfx_engine::Renderer::buildPrograms(std::string shaderDirectory) {
    std::string vertexPath = std::string(shaderDirectory) + "vs_cube.bin";
    std::string fragmentPath = std::string(shaderDirectory) + "fs_cube.bin";

    bgfx::ShaderHandle triangleMeshVertexShader = loadShader(vertexPath.c_str());
    bgfx::ShaderHandle triangleMeshFragmentShader = loadShader(fragmentPath.c_str());

    if (!bgfx::isValid(triangleMeshVertexShader) || !bgfx::isValid(triangleMeshFragmentShader)) {
        std::cerr << "Shader loading failed\n";
        bgfx::shutdown();
        glfwDestroyWindow(info.window);
        glfwTerminate();
        std::exit(-1);
    }

    triangleMeshProgram =
        bgfx::createProgram(triangleMeshVertexShader, triangleMeshFragmentShader, true);

    // other shader
    vertexPath = std::string(shaderDirectory) + "vs_line.bin";
    fragmentPath = std::string(shaderDirectory) + "fs_line.bin";

    bgfx::ShaderHandle lineMeshVertexShader = loadShader(vertexPath.c_str());
    bgfx::ShaderHandle lineMeshFragmentShader = loadShader(fragmentPath.c_str());

    if (!bgfx::isValid(lineMeshVertexShader) || !bgfx::isValid(lineMeshFragmentShader)) {
        std::cerr << "Shader loading failed\n";
        bgfx::shutdown();
        glfwDestroyWindow(info.window);
        glfwTerminate();
        std::exit(-1);
    }

    lineMeshProgram = bgfx::createProgram(lineMeshVertexShader, lineMeshFragmentShader, true);

    // set uniforms
    u_lineColor = bgfx::createUniform("u_lineColor", bgfx::UniformType::Vec4);
}

void obx::gfx_engine::Renderer::setupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsClassic();
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui_ImplGlfw_InitForOther(info.window, true);
}

void obx::gfx_engine::Renderer::initialize() {
    this->setupGLFW();
    this->setupBGFX();
    this->setupImGui();

    imguiRenderer.initialize();

    const char *shaderDirectory = nullptr;

    switch (bgfx::getRendererType()) {
    case bgfx::RendererType::Metal:
        shaderDirectory = "assets/shaders/metal/";
        break;

    case bgfx::RendererType::Vulkan:
        shaderDirectory = "assets/shaders/spirv/";
        break;

    default:
        std::cerr << "Unsupported renderer: " << bgfx::getRendererName(bgfx::getRendererType())
                  << '\n';

        bgfx::shutdown();
        glfwTerminate();
        std::exit(-1);
    }
    this->buildPrograms(shaderDirectory);

    sampler = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
}

void obx::gfx_engine::Renderer::addEntity(std::shared_ptr<Entity> entity) {
    entity->mesh->upload();
    entities.push_back(std::move(entity));
}

void obx::gfx_engine::Renderer::addImguiCallback(void (*imgui_user_callback)()) {
    this->imgui_user_callback = imgui_user_callback;
}

void obx::gfx_engine::Renderer::renderUI() {
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                 ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::Begin("OrbitalX Debug");

    ImGui::Text("Performance");
    ImGui::Separator();

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

    ImGui::Spacing();
    ImGui::Text("Renderer");
    ImGui::Separator();

    ImGui::Text("Backend: %s", bgfx::getRendererName(bgfx::getRendererType()));

    ImGui::Text("Meshes: %zu", entities.size());

    int fbw, fbh;
    glfwGetFramebufferSize(info.window, &fbw, &fbh);
    ImGui::Text("Framebuffer: %d x %d", fbw, fbh);
    ImGui::Text("Cam: %.0f, %.0f, %.0f", camera.position.x, camera.position.y, camera.position.z);

    ImGui::Spacing();
    ImGui::End();

    if (this->imgui_user_callback != nullptr) {
        imgui_user_callback();
    }

    ImGui::Render();
    imguiRenderer.render(ImGui::GetDrawData());
}

void obx::gfx_engine::Renderer::render() {
    glfwPollEvents();

    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int width, height;
    glfwGetFramebufferSize(info.window, &width, &height);

    if (width == 0 || height == 0)
        return;

    if (width != resetWidth || height != resetHeight) {
        bgfx::SwapChain desc;
        desc.nwh = info.nativeWindow;
        desc.ndt = info.nativeDisplay;
        desc.width = static_cast<uint32_t>(width);
        desc.height = static_cast<uint32_t>(height);
        bgfx::reset(BGFX_RESET_VSYNC, &desc);
        resetWidth = width;
        resetHeight = height;
    }

    bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x1a1a1aff, 0.0f, 0);

    float model[16];
    float view[16];
    float projection[16];

    float time = static_cast<float>(glfwGetTime());

    if (camera.useLookDir) {
        bx::mtxLookAt(view, bx::Vec3{0.0f, 0.0f, 0.0f}, camera.lookDir, camera.up);
    } else {
        float cameraTransform[16];
        bx::mtxSRT(cameraTransform, 1.0f, 1.0f, 1.0f, camera.rotation.x, camera.rotation.y,
                   camera.rotation.z, 0.0f, 0.0f, 0.0f);
        bx::mtxInverse(view, cameraTransform);
    }
    // 120 deg fov
    const float nearPlane = 0.01f; // 10 meters (km units)
    const float farPlane = 500000.0f;

    const bool homogeneousDepth = bgfx::getCaps()->homogeneousDepth;

    bx::mtxProj(projection, camera.fov, float(width) / height, farPlane, nearPlane,
                homogeneousDepth);
    bgfx::setViewTransform(0, view, projection);

    for (auto &entity : entities) {
        auto &mesh = entity->mesh;
        auto &texture = entity->texture;

        // camera relative positioning
        const float x = mesh->position.x - camera.position.x;
        const float y = mesh->position.y - camera.position.y;
        const float z = mesh->position.z - camera.position.z;

        mtxSRT(model, mesh->scale.x, mesh->scale.y, mesh->scale.z, mesh->rotation.x,
               mesh->rotation.y, mesh->rotation.z, x, y, z);

        bgfx::setTransform(model);
        bgfx::setVertexBuffer(0, mesh->vertexBuffer);
        if (mesh->meshType == MeshType::TRIANGLE_MESH) {
            bgfx::setIndexBuffer(mesh->indexBuffer);
            bgfx::setTexture(0, sampler, texture);

            bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
                           BGFX_STATE_DEPTH_TEST_GREATER);
            bgfx::submit(0, triangleMeshProgram);
        } else if (mesh->meshType == MeshType::LINE_MESH) {
            // no textures, no index buffer; the line mesh is kinda dookie
            // @todo add a better way to change colors or something
            bgfx::setState(BGFX_STATE_PT_LINESTRIP | BGFX_STATE_DEPTH_TEST_GREATER |
                           BGFX_STATE_WRITE_RGB | BGFX_STATE_BLEND_ALPHA);

            // bind uniform
            bgfx::setUniform(u_lineColor, mesh->lineColor);

            // i think you keep it the same id to ensure the depth buffer is the same but don't
            // quote me on that (imgui is 255 btw)
            bgfx::submit(0, lineMeshProgram);
        }
    }

    this->renderUI();

    bgfx::frame();
}
