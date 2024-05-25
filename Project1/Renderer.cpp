#include "renderer.h"
#include "scene.h"
#include "mesh.h"
#include <iostream>
#include <random>

Renderer::Renderer(int width, int height)
    : width(width), height(height), gbuffer(width, height),
    geometryPassShader("geometry_pass.vert", "geometry_pass.frag"),
    lightingPassShader("lighting_pass.vert", "lighting_pass.frag"),
    ssaoShader("ssao.vert", "ssao.frag"),
    ssaoBlurShader("ssao.vert", "ssao_blur.frag"),
    shadowShader("shadow.vert", "shadow.frag") { // Initialize shadow shader
    InitQuad();
    InitSSAO();
    InitShadowMap();
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteFramebuffers(1, &ssaoFBO);
    glDeleteFramebuffers(1, &ssaoBlurFBO);
    glDeleteTextures(1, &ssaoColorBuffer);
    glDeleteTextures(1, &ssaoColorBufferBlur);
    glDeleteTextures(1, &noiseTexture);
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
}

void Renderer::RenderScene(GLuint vao, int vertexCount, Camera& camera, const Scene& scene) {
    ShadowPass(scene);            // First pass: render depth into shadow map
    GeometryPass(vao, vertexCount, camera, scene); // Geometry pass
    SSAOPass(camera);             // SSAO pass
    LightingPass(camera, scene);  // Lighting pass
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
        glm::mat4 model = glm::mat4(1.0f);
        geometryPassShader.setMat4("model", model);
        mesh.Draw(geometryPassShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SSAOPass(Camera& camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    ssaoShader.use();
    for (unsigned int i = 0; i < 64; ++i) {
        ssaoShader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
    }
    ssaoShader.setMat4("projection", camera.GetProjectionMatrix((float)width / (float)height));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer.GetPositionTexture());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer.GetNormalTexture());
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    ssaoBlurShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
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

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    lightingPassShader.setInt("ssao", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    lightingPassShader.setInt("shadowMap", 4);

    lightingPassShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

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


void Renderer::ShadowPass(const Scene& scene) {
    glViewport(0, 0, 4096, 4096); // Set viewport to shadow map size
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    shadowShader.use();

    glm::mat4 lightProjection, lightView;
    float near_plane = 1.0f, far_plane = 50.0f; // Adjust far_plane based on scene size
    lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
    glm::vec3 lightPos = scene.GetDirectionalLight().position; // Make sure light position is correctly set
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    for (const auto& mesh : scene.GetMeshes()) {
        glm::mat4 model = glm::mat4(1.0f);
        shadowShader.setMat4("model", model);
        mesh.Draw(shadowShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height); // Reset viewport
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

void Renderer::InitSSAO() {
    // SSAO framebuffer
    glGenFramebuffers(1, &ssaoFBO);
    glGenFramebuffers(1, &ssaoBlurFBO);

    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

    // SSAO blur color buffer
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Generate sample kernel
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator)
        );
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0;

        // Scale samples s.t. they are more aligned to the origin
        scale = 0.1f + scale * scale * (1.0f - 0.1f);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // Generate noise texture
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f
        );
        ssaoNoise.push_back(noise);
    }

    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Renderer::InitShadowMap() {
    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Changed from GL_NEAREST to GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Changed from GL_NEAREST to GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);  // No color buffer is drawn to
    glReadBuffer(GL_NONE);  // No color buffer is read from

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind the framebuffer
}
