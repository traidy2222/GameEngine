#ifndef RENDERER_H
#define RENDERER_H

#include "shader.h"
#include "camera.h"
#include "scene.h"
#include "mesh.h"
#include "gbuffer.h"
#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();
    void RenderScene(GLuint vao, int vertexCount, Camera& camera, const Scene& scene);

private:
    int width, height;
    Shader geometryPassShader;
    Shader lightingPassShader;
    Shader ssaoShader;
    Shader ssaoBlurShader;
    Shader shadowShader; // Shadow map shader

    GBuffer gbuffer;

    void GeometryPass(GLuint vao, int vertexCount, Camera& camera, const Scene& scene);
    void LightingPass(const Camera& camera, const Scene& scene);
    void SSAOPass(Camera& camera);
    void ShadowPass(const Scene& scene);

    void InitQuad();
    void InitSSAO();
    void InitShadowMap();

    // Quad VAO and VBO
    GLuint quadVAO, quadVBO;

    // SSAO Framebuffers and Textures
    GLuint ssaoFBO, ssaoBlurFBO;
    GLuint ssaoColorBuffer, ssaoColorBufferBlur, noiseTexture;

    // SSAO Kernel
    std::vector<glm::vec3> ssaoKernel;

    // Shadow map FBO and texture
    GLuint depthMapFBO, depthMap;
    glm::mat4 lightSpaceMatrix;
};

#endif // RENDERER_H
