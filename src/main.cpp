#include <iostream>

#include <GLFW/glfw3.h>

// I do not have any intention to support nonlinux builds (yet)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

// bgfx
#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace obx {

struct GLFWInfo {
    GLFWwindow *window;
    void *nativeWindow;
    void *nativeDisplay;
};

GLFWInfo setup_glfw() {
    if (!glfwInit()) {
        std::cout << "glfw init failed -1";
        std::exit(-1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window =
        glfwCreateWindow(1280, 720, "OribtalX", nullptr, nullptr);

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

    return {window, nativeDisplay, nativeWindow};
}

void init_bgfx(GLFWInfo info) {
    bgfx::Init init;

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
}

} // namespace obx

int main() {
    std::cout << "Build system working?! " << std::endl;

    obx::GLFWInfo glfwInfo = obx::setup_glfw();

    return 0;
}
