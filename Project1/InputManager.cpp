#include "inputmanager.h"

InputManager::InputManager() : window(nullptr), mousePosition{ 0.0, 0.0 }, mouseDelta{ 0.0, 0.0 }, lastMousePosition{ 0.0, 0.0 } {}

InputManager::~InputManager() {}

void InputManager::setWindow(GLFWwindow* inWindow) {
    window = inWindow;
    glfwSetWindowUserPointer(window, this);

    glfwSetKeyCallback(window, keyCallbackWrapper);
    glfwSetMouseButtonCallback(window, mouseButtonCallbackWrapper);
    glfwSetCursorPosCallback(window, cursorPositionCallbackWrapper);
}

void InputManager::update() {
    mouseDelta = std::make_pair(mousePosition.first - lastMousePosition.first, mousePosition.second - lastMousePosition.second);
    lastMousePosition = mousePosition;
}

bool InputManager::isKeyPressed(int key) const {
    auto it = keyStates.find(key);
    return it != keyStates.end() && it->second;
}

bool InputManager::isMouseButtonPressed(int button) const {
    auto it = mouseButtonStates.find(button);
    return it != mouseButtonStates.end() && it->second;
}

std::pair<double, double> InputManager::getMousePosition() const {
    return mousePosition;
}

std::pair<double, double> InputManager::getMouseDelta() const {
    return mouseDelta;
}

void InputManager::keyCallback(int key, int scancode, int action, int mods) {
    keyStates[key] = (action != GLFW_RELEASE);
}

void InputManager::mouseButtonCallback(int button, int action, int mods) {
    mouseButtonStates[button] = (action != GLFW_RELEASE);
}

void InputManager::cursorPositionCallback(double xpos, double ypos) {
    mousePosition = std::make_pair(xpos, ypos);
}

void InputManager::keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (inputManager) {
        inputManager->keyCallback(key, scancode, action, mods);
    }
}

void InputManager::mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods) {
    InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (inputManager) {
        inputManager->mouseButtonCallback(button, action, mods);
    }
}

void InputManager::cursorPositionCallbackWrapper(GLFWwindow* window, double xpos, double ypos) {
    InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (inputManager) {
        inputManager->cursorPositionCallback(xpos, ypos);
    }
}
