#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <utility>
#include <vector>

class InputManager {
public:
    InputManager();
    ~InputManager();

    void update();

    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    std::pair<double, double> getMousePosition() const;
    std::pair<double, double> getMouseDelta() const;

    void setWindow(GLFWwindow* window);

private:
    void keyCallback(int key, int scancode, int action, int mods);
    void mouseButtonCallback(int button, int action, int mods);
    void cursorPositionCallback(double xpos, double ypos);

    static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallbackWrapper(GLFWwindow* window, double xpos, double ypos);

    GLFWwindow* window;
    std::unordered_map<int, bool> keyStates;
    std::unordered_map<int, bool> mouseButtonStates;
    std::pair<double, double> mousePosition;
    std::pair<double, double> mouseDelta;
    std::pair<double, double> lastMousePosition;
};

#endif // INPUTMANAGER_H
