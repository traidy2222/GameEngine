#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();

    bool shouldClose() const;
    void swapBuffers() const;
    void pollEvents() const;
    GLFWwindow* getGLFWwindow() const;

private:
    const char* title;
    int width;
    int height;
    GLFWwindow* window;

    bool init();
    void cleanup();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

#endif // WINDOW_H
