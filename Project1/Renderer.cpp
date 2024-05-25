#include "renderer.h"
#include "scene.h"
#include "mesh.h" // Include the Mesh header
#include <iostream>

Renderer::Renderer(int width, int height)
    : width(width), height(height), gbuffer(width, height),
    geometryPassShader("geometry_pass.vert", "geometry_pass.frag"),
    lightingPassShader("lighting_pass.vert", "lighting_pass.frag") {
    InitQuad();
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}

void Renderer::RenderScene(GLuint vao, int vertexCount, Camera& camera, const Scene& scene) {
    GeometryPass(vao, vertexCount, camera, scene);
    LightingPass(camera, scene);
}

void Renderer::GeometryPass(GLuint vao, int vertexCount, Camera& camera, const Scene& scene) {
    gbuffer.BindForWriting();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    geometryPassShader.use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjectionMatrix((float)width / (float)height);

    geometryPassShader.setMat4("view", view);
    geometryPassShader.setMat4("projection", projection);

    for (const auto& mesh : scene.GetMeshes()) {
        glm::mat4 model = glm::mat4(1.0f); // Assuming default model matrix, you can update this as needed
        geometryPassShader.setMat4("model", model);
        mesh.Draw(geometryPassShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::LightingPass(const Camera& camera, const Scene& scene) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lightingPassShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer.GetPositionTexture());
    lightingPassShader.setInt("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer.GetNormalTexture());
    lightingPassShader.setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer.GetAlbedoTexture());
    lightingPassShader.setInt("gAlbedoSpec", 2);

    // Set directional light uniforms
    const auto& directionalLight = scene.GetDirectionalLight();
    lightingPassShader.setVec3("dirLight.direction", directionalLight.direction);
    lightingPassShader.setVec3("dirLight.ambient", directionalLight.ambient);
    lightingPassShader.setVec3("dirLight.diffuse", directionalLight.diffuse);
    lightingPassShader.setVec3("dirLight.specular", directionalLight.specular);

    // Set point lights uniforms
    const auto& pointLights = scene.GetPointLights();
    lightingPassShader.setInt("numPointLights", pointLights.size());
    for (unsigned int i = 0; i < pointLights.size(); i++) {
        lightingPassShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLights[i].position);
        lightingPassShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", pointLights[i].ambient);
        lightingPassShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", pointLights[i].diffuse);
        lightingPassShader.setVec3("pointLights[" + std::to_string(i) + "].specular", pointLights[i].specular);
        lightingPassShader.setFloat("pointLights[" + std::to_string(i) + "].constant", pointLights[i].constant);
        lightingPassShader.setFloat("pointLights[" + std::to_string(i) + "].linear", pointLights[i].linear);
        lightingPassShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", pointLights[i].quadratic);
    }

    // Set spot lights uniforms
    const auto& spotLights = scene.GetSpotLights();
    lightingPassShader.setInt("numSpotLights", spotLights.size());
    for (unsigned int i = 0; i < spotLights.size(); i++) {
        lightingPassShader.setVec3("spotLights[" + std::to_string(i) + "].position", spotLights[i].position);
        lightingPassShader.setVec3("spotLights[" + std::to_string(i) + "].direction", spotLights[i].direction);
        lightingPassShader.setVec3("spotLights[" + std::to_string(i) + "].ambient", spotLights[i].ambient);
        lightingPassShader.setVec3("spotLights[" + std::to_string(i) + "].diffuse", spotLights[i].diffuse);
        lightingPassShader.setVec3("spotLights[" + std::to_string(i) + "].specular", spotLights[i].specular);
        lightingPassShader.setFloat("spotLights[" + std::to_string(i) + "].constant", spotLights[i].constant);
        lightingPassShader.setFloat("spotLights[" + std::to_string(i) + "].linear", spotLights[i].linear);
        lightingPassShader.setFloat("spotLights[" + std::to_string(i) + "].quadratic", spotLights[i].quadratic);
        lightingPassShader.setFloat("spotLights[" + std::to_string(i) + "].cutOff", spotLights[i].cutOff);
        lightingPassShader.setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", spotLights[i].outerCutOff);
    }

    // Set the camera position uniform
    lightingPassShader.setVec3("viewPos", camera.GetPosition());

    // Render a quad for the lighting pass
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::InitQuad() {
    float quadVertices[] = {
        // Positions        // Texture Coords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}
