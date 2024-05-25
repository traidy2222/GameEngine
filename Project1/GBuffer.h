#ifndef GBUFFER_H
#define GBUFFER_H

#include <GL/glew.h>

class GBuffer {
public:
    GBuffer(int width, int height);
    ~GBuffer();

    void BindForWriting();
    void BindForReading();

    GLuint GetPositionTexture();
    GLuint GetNormalTexture();
    GLuint GetAlbedoTexture();

private:
    GLuint fbo;
    GLuint positionTexture;
    GLuint normalTexture;
    GLuint albedoTexture;
    GLuint depthRenderBuffer;

    int width, height;

    bool Init();
};

#endif // GBUFFER_H
