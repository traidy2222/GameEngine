#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;

    void ProcessKeyboardInput(float deltaTime, bool moveForward, bool moveBackward, bool moveLeft, bool moveRight);
    void ProcessMouseMovement(float xoffset, float yoffset);

    // Getter methods for position and front
    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetFront() const { return front; }

private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;

    void updateCameraVectors();
};

#endif // CAMERA_H
