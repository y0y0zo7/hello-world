/**
 * Desktop/Web platform bridge. Mouse press, drag, and release are forwarded to
 * the same model callbacks used by Android touch events.
 */

#include "Platform.h"
#include "Renderer.h"

namespace {
GLFWwindow* window = nullptr;
bool pointerDown = false;

void mouseButtonCallback(GLFWwindow* currentWindow, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(currentWindow, &x, &y);

    if (action == GLFW_PRESS) {
        pointerDown = true;
        Renderer::Instance().TouchEventDown(static_cast<float>(x), static_cast<float>(y));
    } else if (action == GLFW_RELEASE) {
        pointerDown = false;
        Renderer::Instance().TouchEventRelease(static_cast<float>(x), static_cast<float>(y));
    }
}

void cursorPositionCallback(GLFWwindow*, double x, double y)
{
    if (pointerDown) {
        Renderer::Instance().TouchEventMove(static_cast<float>(x), static_cast<float>(y));
    }
}

void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
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

    window = glfwCreateWindow(800, 700, "Assignment 2 - 3D Fan", nullptr, nullptr);
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
    Renderer::Instance().resize(width, height);
    return true;
}

void renderFrame()
{
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
    LOGI("3D Fan: click to toggle; drag to boost blade speed");
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
