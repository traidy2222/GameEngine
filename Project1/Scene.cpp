#include "scene.h"

void Scene::AddMesh(const Mesh& mesh) {
    meshes.push_back(mesh);
}

void Scene::SetDirectionalLight(const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular) {
    directionalLight.direction = direction;
    directionalLight.ambient = ambient;
    directionalLight.diffuse = diffuse;
    directionalLight.specular = specular;
}

void Scene::AddPointLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic) {
    PointLight pointLight = { position, ambient, diffuse, specular, constant, linear, quadratic };
    pointLights.push_back(pointLight);
}

void Scene::SetSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff) {
    SpotLight spotLight = { position, direction, ambient, diffuse, specular, constant, linear, quadratic, cutOff, outerCutOff };
    spotLights.push_back(spotLight);
}
