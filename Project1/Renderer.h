#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <glm/glm.hpp>
#include "gbuffer.h"
#include "shader.h"
#include "camera.h"
#include "scene.h"

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void RenderScene(GLuint vao, int vertexCount, Camera& camera, const Scene& scene);
    void Resize(int newWidth, int newHeight); // Add this method

private:
    int width;
    int height;
    GBuffer gbuffer;
    Shader geometryPassShader;
    Shader lightingPassShader;
    Shader ssaoShader;
    Shader ssaoBlurShader;
    Shader shadowShader; // Add a shader variable for shadow mapping
    GLuint quadVAO;
    GLuint quadVBO;
    GLuint ssaoFBO;
    GLuint ssaoBlurFBO;
    GLuint ssaoColorBuffer;
    GLuint ssaoColorBufferBlur;
    GLuint noiseTexture;
    GLuint depthMapFBO;
    GLuint depthMap;
    std::vector<glm::vec3> ssaoKernel;
    glm::mat4 lightSpaceMatrix;

    void InitQuad();
    void InitSSAO();
    void InitShadowMap();
    void GeometryPass(GLuint vao, int vertexCount, Camera& camera, const Scene& scene);
    void SSAOPass(Camera& camera);
    void LightingPass(const Camera& camera, const Scene& scene);
    void ShadowPass(const Scene& scene);
};

#endif // RENDERER_H
