/**
 * @file main.cpp
 * @brief Bridges Desktop/Web window events to the shared HUD-first renderer.
 * @author Zoey <t.zoey@digipen.edu>
 */

#include "Platform.h"
#include "Renderer.h"

namespace {
GLFWwindow* window = nullptr;
bool pointerDown = false;
int framebufferWidth = 0;
int framebufferHeight = 0;

void cursorToFramebuffer(GLFWwindow* currentWindow, double& x, double& y)
{
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(currentWindow, &windowWidth, &windowHeight);
    if (windowWidth > 0 && windowHeight > 0
        && framebufferWidth > 0 && framebufferHeight > 0) {
        x *= static_cast<double>(framebufferWidth) / static_cast<double>(windowWidth);
        y *= static_cast<double>(framebufferHeight) / static_cast<double>(windowHeight);
    }
}

void mouseButtonCallback(GLFWwindow* currentWindow, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(currentWindow, &x, &y);
    cursorToFramebuffer(currentWindow, x, y);

    if (action == GLFW_PRESS) {
        pointerDown = true;
        Renderer::Instance().TouchEventDown(static_cast<float>(x), static_cast<float>(y));
    } else if (action == GLFW_RELEASE) {
        pointerDown = false;
        Renderer::Instance().TouchEventRelease(static_cast<float>(x), static_cast<float>(y));
    }
}

void cursorPositionCallback(GLFWwindow* currentWindow, double x, double y)
{
    if (pointerDown) {
        cursorToFramebuffer(currentWindow, x, y);
        Renderer::Instance().TouchEventMove(static_cast<float>(x), static_cast<float>(y));
    }
}

void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    framebufferWidth = width;
    framebufferHeight = height;
    Renderer::Instance().resize(width, height);
}

bool initializeWindow()
{
    if (!glfwInit()) {
        LOGE("glfwInit failed");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(900, 720, "Programming Quiz 3 - Fan Dashboard HUD", nullptr, nullptr);
    if (!window) {
        LOGE("glfwCreateWindow failed");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
#ifndef PLATFORM_EMSCRIPTEN
    glfwSwapInterval(1);
#endif
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    return true;
}

bool initializeScene()
{
#ifndef PLATFORM_EMSCRIPTEN
    glewExperimental = GL_TRUE;
    const GLenum glewStatus = glewInit();
    // GLEW may emit GL_INVALID_ENUM while probing a core/ES context.
    glGetError();
    if (glewStatus != GLEW_OK) {
        LOGE("glewInit failed: %s", glewGetErrorString(glewStatus));
        return false;
    }
#endif

    if (!Renderer::Instance().initializeRenderer()) return false;
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSizeCallback(window, width, height);
    return true;
}

void renderFrame()
{
    // Web fullscreen can change the canvas drawing-buffer size without a GLFW
    // callback. Polling also makes resize handling reliable across browsers.
    int currentWidth = 0;
    int currentHeight = 0;
    glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
    if (currentWidth != framebufferWidth || currentHeight != framebufferHeight) {
        framebufferSizeCallback(window, currentWidth, currentHeight);
    }

    Renderer::Instance().render();
    glfwSwapBuffers(window);
    glfwPollEvents();
}
} // namespace

#ifdef PLATFORM_EMSCRIPTEN
void browserMainLoop()
{
    if (glfwWindowShouldClose(window)) {
        emscripten_cancel_main_loop();
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }
    renderFrame();
}
#endif

int main()
{
    LOGI("Fan Dashboard HUD: use the four buttons; click the fan to pick a part");
    if (!initializeWindow() || !initializeScene()) return 1;

#ifdef PLATFORM_EMSCRIPTEN
    emscripten_set_main_loop(browserMainLoop, 0, 1);
#else
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        renderFrame();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
#endif
    return 0;
}
