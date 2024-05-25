#ifndef RENDERER_H
#define RENDERER_H

#include "shader.h"
#include "camera.h"
#include "scene.h"
#include "mesh.h"
#include "gbuffer.h"
#include <GL/glew.h>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();
    void RenderScene(GLuint vao, int vertexCount, Camera& camera, const Scene& scene);

private:
    int width, height;
    Shader geometryPassShader;
    Shader lightingPassShader;
    GBuffer gbuffer;

    void GeometryPass(GLuint vao, int vertexCount, Camera& camera, const Scene& scene);
    void LightingPass(const Camera& camera, const Scene& scene);

    // For quad rendering in the lighting pass
    unsigned int quadVAO, quadVBO;
    void InitQuad();
};

#endif // RENDERER_H
