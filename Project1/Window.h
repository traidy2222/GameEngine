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

    GLFWwindow* getGLFWwindow() const; // Add this getter function

private:
    bool init();
    void cleanup();

    const char* title;
    int width;
    int height;
    GLFWwindow* window;
};

#endif // WINDOW_H
