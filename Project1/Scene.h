#ifndef SCENE_H
#define SCENE_H

#include "gbuffer.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <vector>

class Scene {
public:
    void AddMesh(const Mesh& mesh);
    void SetDirectionalLight(const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
    void AddPointLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic);
    void SetSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff);

    const auto& GetMeshes() const { return meshes; }
    const auto& GetDirectionalLight() const { return directionalLight; }
    const auto& GetPointLights() const { return pointLights; }
    const auto& GetSpotLights() const { return spotLights; }

private:
    std::vector<Mesh> meshes;
    struct DirectionalLight {
        glm::vec3 direction;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };
    struct PointLight {
        glm::vec3 position;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float constant;
        float linear;
        float quadratic;
    };
    struct SpotLight {
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float constant;
        float linear;
        float quadratic;
        float cutOff;
        float outerCutOff;
    };

    DirectionalLight directionalLight;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;
};

#endif // SCENE_H
