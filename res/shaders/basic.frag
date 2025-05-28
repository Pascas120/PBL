#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec2 Velocity;


in vec2 texCoords;
in vec3 fragPos;
in vec3 normal;
in vec4 fragPosLightSpace;

uniform vec3 diffuse;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

layout(std140) uniform Camera
{
    vec3 viewPos;
    mat4 view;
    mat4 invView;
    mat4 projection;
    mat4 invProjection;
    mat4 viewProjection;
    mat4 invViewProjection;
    mat4 lightProjection;
    mat4 lightView;
    mat4 prevViewProjection;
};

#define MAX_POINT_LIGHTS 100
#define MAX_DIRECTIONAL_LIGHTS 10

struct PointLight {
    vec3 position;
    vec4 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    vec3 direction;
    vec4 color;
    float intensity;
};

layout(std140) uniform Lights
{
    vec4 ambientColor;
    PointLight pointLights[MAX_POINT_LIGHTS];
    int pointLightCount;
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
    int directionalLightCount;
};

vec3 CalculatePointLight(PointLight light, vec3 norm, vec3 viewDir);
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 norm, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace);
float packColor(vec3 color);

void main()
{
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 lighting = vec3(0.0);


    for (int i = 0; i < pointLightCount; i++) {
        lighting += CalculatePointLight(pointLights[i], norm, viewDir);
    }

    for (int i = 0; i < directionalLightCount; i++) {
        lighting += CalculateDirectionalLight(directionalLights[i], norm, viewDir);
    }


    vec4 texColor = texture(texture_diffuse1, texCoords);
    vec4 debugColor = texture(shadowMap, texCoords);
    float shadow = ShadowCalculation(vec4(fragPosLightSpace));
    vec4 finalColor = texColor * (vec4(lighting, 1.0) * (1.0 - shadow) + ambientColor);

    FragColor = finalColor;
    //FragColor = vec4(vec3(shadow), 1.0); // For debugging shadow
    Velocity = vec2(0.0, 0.0);
}

////////////////////////////////////////


vec3 CalculatePointLight(PointLight light, vec3 norm, vec3 viewDir) {
    vec3 lightDir = light.position - fragPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * light.color.rgb;

    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    return (diffuse + specular) * light.intensity * attenuation;
}


vec3 CalculateDirectionalLight(DirectionalLight light, vec3 norm, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * light.color.rgb;

    return (diffuse + specular) * light.intensity;
}
float ShadowCalculation(vec4 fragPosLightSpace) {
    if( fragPosLightSpace.w <= 0.0) {
        return 1.0;
    }
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;


    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

float packColor(vec3 color) {   return (color.r + (color.g*256.) + (color.b*256.*256.)) / (256.*256.*256.); }
