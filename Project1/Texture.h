#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>  // Use GLEW instead of GLAD
#include <string>
#include <iostream>

class Texture {
public:
    unsigned int id;
    std::string type;
    std::string path;

    Texture(const std::string& path, const std::string& type);

    void bind(unsigned int unit) const;

private:
    void loadTexture(const std::string& path);
};

#endif // TEXTURE_H
