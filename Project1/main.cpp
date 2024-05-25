#include <iostream>
#include "window.h"
#include "inputmanager.h"
#include "renderer.h"
#include "camera.h"
#include "scene.h"
#include "mesh.h"
#include "texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Define the global renderer pointer
Renderer* renderer = nullptr;

// Vertex data for a cube
std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
    {{0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    {{0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}
};

// Index data for a cube
std::vector<unsigned int> indices = {
    0, 1, 2, 2, 3, 0,  // -Z
    4, 5, 6, 6, 7, 4,  // +Z
    8, 9, 10, 10, 11, 8,  // -X
    12, 13, 14, 14, 15, 12,  // +X
    16, 17, 18, 18, 19, 16,  // -Y
    20, 21, 22, 22, 23, 20   // +Y
};

int main() {
    Window window("Deferred Rendering", 800, 600);
    InputManager inputManager;
    inputManager.setWindow(window.getGLFWwindow());

    glewExperimental = GL_TRUE;
    glewInit();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Create the renderer
    renderer = new Renderer(800, 600);
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    Scene scene;


    // Load textures
    Texture diffuseTexture("Assets/Textures/StoneFloor/BC.jpg", "texture_diffuse");
    Texture specularTexture("Assets/Textures/StoneFloor/AO.jpg", "texture_specular");
    std::vector<Texture> textures = { diffuseTexture, specularTexture };

    // Create a cube mesh and add it to the scene
    Mesh cubeMesh(vertices, indices, textures);
    scene.AddMesh(cubeMesh);

    // Set up lights

    glm::vec3 lightPos(0.0f, 10.0f, 0.0f);
    glm::vec3 lightDir(-0.2f, -1.0f, -0.3f);
    glm::vec3 ambient(0.05f, 0.05f, 0.05f);
    glm::vec3 diffuse(0.4f, 0.4f, 0.4f);
    glm::vec3 specular(0.5f, 0.5f, 0.5f);
    scene.SetDirectionalLight(lightPos, lightDir, ambient, diffuse, specular);
    //scene.AddPointLight(glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f);
    //scene.SetSpotLight(camera.GetPosition(), camera.GetFront(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.09f, 0.032f, glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)));

    // Ensure the correct winding order
    glFrontFace(GL_CCW);

    // For debugging purposes, you can disable face culling to ensure all faces are rendered
    glDisable(GL_CULL_FACE); // Comment or remove this line after debugging

    while (!window.shouldClose()) {
        float currentFrame = glfwGetTime();
        static float lastFrame = 0.0f;
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        inputManager.update();

        if (inputManager.isKeyPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window.getGLFWwindow(), true);
        }
        if (inputManager.isKeyPressed(GLFW_KEY_W)) {
            camera.ProcessKeyboardInput(deltaTime, true, false, false, false);
        }
        if (inputManager.isKeyPressed(GLFW_KEY_S)) {
            camera.ProcessKeyboardInput(deltaTime, false, true, false, false);
        }
        if (inputManager.isKeyPressed(GLFW_KEY_A)) {
            camera.ProcessKeyboardInput(deltaTime, false, false, true, false);
        }
        if (inputManager.isKeyPressed(GLFW_KEY_D)) {
            camera.ProcessKeyboardInput(deltaTime, false, false, false, true);
        }

        static double lastX = 400, lastY = 300;
        std::pair<double, double> mousePos = inputManager.getMousePosition();
        double xpos = mousePos.first;
        double ypos = mousePos.second;
        camera.ProcessMouseMovement(xpos - lastX, lastY - ypos);
        lastX = xpos;
        lastY = ypos;

        renderer->RenderScene(0 /* Your VAO id */, 36 /* Vertex count */, camera, scene);

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
