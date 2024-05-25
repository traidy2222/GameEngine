#include "window.h"
#include <iostream>

Window::Window(const char* title, int width, int height)
    : title(title), width(width), height(height), window(nullptr) {
    if (!init()) {
        std::cerr << "Failed to initialize window." << std::endl;
    }
}

Window::~Window() {
    cleanup();
}

bool Window::init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        return false;
    }

    glViewport(0, 0, width, height);
    return true;
}

void Window::cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::swapBuffers() const {
    glfwSwapBuffers(window);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

GLFWwindow* Window::getGLFWwindow() const { // Implement the getter function
    return window;
}
