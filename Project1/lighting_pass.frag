#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight dirLight;

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

#define MAX_POINT_LIGHTS 16
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

#define MAX_SPOT_LIGHTS 16
uniform int numSpotLights;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform vec3 viewPos;

vec3 poissonDisk[16] = vec3[](
    vec3(-0.94201624, -0.39906216, 0.0), vec3(0.94558609, -0.76890725, 0.0),
    vec3(-0.094184101, -0.92938870, 0.0), vec3(0.34495938, 0.29387760, 0.0),
    vec3(-0.91588581, 0.45771432, 0.0), vec3(-0.81544232, -0.87912464, 0.0),
    vec3(-0.38277543, 0.27676845, 0.0), vec3(0.97484398, 0.75648379, 0.0),
    vec3(0.44323325, -0.97511554, 0.0), vec3(0.53742981, -0.47373420, 0.0),
    vec3(-0.26496911, -0.41893023, 0.0), vec3(0.79197514, 0.19090188, 0.0),
    vec3(-0.24188840, 0.99706507, 0.0), vec3(-0.81409955, 0.91437590, 0.0),
    vec3(0.19984126, 0.78641367, 0.0), vec3(0.14383161, -0.14100790, 0.0)
);

float PenumbraSize(vec3 projCoords, float currentDepth, float lightSize)
{
    float searchRadius = 32.0 / 4096.0; // Assuming the shadow map resolution is 4096x4096
    float blockerDepthSum = 0.0;
    int blockerCount = 0;
    
    for (int i = 0; i < 16; ++i) {
        vec2 offset = poissonDisk[i].xy * searchRadius;
        float sampleDepth = texture(shadowMap, projCoords.xy + offset).r;
        if (sampleDepth < currentDepth) {
            blockerDepthSum += sampleDepth;
            blockerCount++;
        }
    }

    if (blockerCount > 0) {
        float avgBlockerDepth = blockerDepthSum / blockerCount;
        float penumbraSize = lightSize * (currentDepth - avgBlockerDepth) / avgBlockerDepth;
        return penumbraSize;
    }

    return 0.0;
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, float lightSize)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, normalize(dirLight.direction))), 0.005);
    float shadow = 0.0;

    float penumbraSize = PenumbraSize(projCoords, currentDepth, lightSize);
    penumbraSize = clamp(penumbraSize, 1.0 / 4096.0, 1.0);

    for (int i = 0; i < 16; ++i) {
        vec2 offset = poissonDisk[i].xy * penumbraSize;
        float sampleDepth = texture(shadowMap, projCoords.xy + offset).r;
        shadow += currentDepth - bias > sampleDepth ? 1.0 : 0.0;
    }
    shadow /= 16.0;

    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 fragPos, vec3 albedo, float spec, float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specStrength = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * specStrength * spec;
    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * diff * albedo;
    return (ambient + diffuse + specular) * (1.0 - shadow);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos, vec3 albedo, float spec, float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specStrength = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * specStrength * spec;
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * diff * albedo;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular) * (1.0 - shadow);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos, vec3 albedo, float spec, float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specStrength = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * specStrength * spec;
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * diff * albedo;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular) * (1.0 - shadow);
}

void main()
{
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float spec = texture(gAlbedoSpec, TexCoords).a;
    float ao = texture(ssao, TexCoords).r;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace, normal, 0.05); // Adjust the light size as needed

    vec3 result = vec3(0.0);
    result += CalculateDirectionalLight(dirLight, normal, viewDir, fragPos, albedo, spec, shadow);

    for (int i = 0; i < numPointLights; i++) {
        result += CalculatePointLight(pointLights[i], normal, viewDir, fragPos, albedo, spec, shadow);
    }

    for (int i = 0; i < numSpotLights; i++) {
        result += CalculateSpotLight(spotLights[i], normal, viewDir, fragPos, albedo, spec, shadow);
    }

    result *= ao;
    FragColor = vec4(result, 1.0);
}
