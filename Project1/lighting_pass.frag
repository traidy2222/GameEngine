#version 330 core

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

#define NR_POINT_LIGHTS 4
#define NR_SPOT_LIGHTS 1

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gMetallicRoughnessAO;

uniform vec3 viewPos;

uniform DirectionalLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

uniform int numPointLights;
uniform int numSpotLights;

const float PI = 3.14159265359;

// Function prototypes
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic);

// GGX/Trowbridge-Reitz Normal Distribution Function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Schlick-GGX Geometry Function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// Smith's Method for combined geometry term
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    vec3 MetallicRoughnessAO = texture(gMetallicRoughnessAO, TexCoords).rgb;

    float Metallic = MetallicRoughnessAO.r;
    float Roughness = MetallicRoughnessAO.g;
    float AO = MetallicRoughnessAO.b;

    vec3 viewDir = normalize(viewPos - FragPos);

    // Initialize lighting variables
    vec3 Lo = vec3(0.0);

    // Directional light
    Lo += CalcDirLight(dirLight, Normal, viewDir, Albedo, Roughness, Metallic);

    // Point lights
    for(int i = 0; i < numPointLights; i++)
        Lo += CalcPointLight(pointLights[i], Normal, FragPos, viewDir, Albedo, Roughness, Metallic);

    // Spot lights
    for(int i = 0; i < numSpotLights; i++)
        Lo += CalcSpotLight(spotLights[i], Normal, FragPos, viewDir, Albedo, Roughness, Metallic);

    vec3 ambient = Albedo * 0.03 * AO;  // Ambient Lighting (using AO)
    vec3 color = ambient + Lo;

    // Apply gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic)
{
    vec3 lightDir = normalize(-light.direction); // Normalize light direction
    vec3 halfDir = normalize(lightDir + viewDir);

    vec3 radiance = light.color;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, halfDir, roughness); // Normal Distribution Function
    float G = GeometrySmith(normal, viewDir, lightDir, roughness); // Geometry Function
    vec3 F = fresnelSchlick(max(dot(halfDir, viewDir), 0.0), vec3(0.04)); // Fresnel term, assuming F0 = 0.04 (non-metallic)

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL + 0.001; // Avoid division by zero
    vec3 specular = numerator / denominator;

    vec3 diffuse = kD * albedo / PI; // Lambertian diffuse
    vec3 ambient = albedo * 0.03; // Simple ambient lighting

    vec3 Lo = (diffuse + specular) * radiance * NdotL + ambient;
    return Lo;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * (distance * distance));

    vec3 radiance = light.color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, halfDir, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfDir, viewDir), 0.0), vec3(0.04));

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL + 0.001;
    vec3 specular = numerator / denominator;

    vec3 diffuse = kD * albedo / PI; // Lambertian diffuse
    vec3 ambient = albedo * 0.03; // Simple ambient lighting

    vec3 Lo = (diffuse + specular) * radiance * NdotL + ambient;
    return Lo;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic)
{
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * (distance * distance));

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 radiance = light.color * attenuation * intensity;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, halfDir, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfDir, viewDir), 0.0), vec3(0.04));

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL + 0.001;
    vec3 specular = numerator / denominator;

    vec3 diffuse = kD * albedo / PI; // Lambertian diffuse
    vec3 ambient = albedo * 0.03; // Simple ambient lighting

    vec3 Lo = (diffuse + specular) * radiance * NdotL + ambient;
    return Lo;
}
