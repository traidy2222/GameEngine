#include "scene.h"

void Scene::AddMesh(const Mesh& mesh) {
    meshes.push_back(mesh);
}

void Scene::SetDirectionalLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular) {
    directionalLight.position = position;
    directionalLight.direction = direction;
    directionalLight.ambient = ambient;
    directionalLight.diffuse = diffuse;
    directionalLight.specular = specular;
}

void Scene::AddPointLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic) {
    pointLights.push_back({ position, ambient, diffuse, specular, constant, linear, quadratic });
}

void Scene::AddSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff) {
    spotLights.push_back({ position, direction, ambient, diffuse, specular, constant, linear, quadratic, cutOff, outerCutOff });
}
