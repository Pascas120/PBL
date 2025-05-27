#version 330 core
out vec4 FragColor;

in vec2 texCoords;
in vec3 fragPos;
in vec3 normal;
in vec3 anormal;

uniform vec3 diffuse;

uniform sampler2D texture_diffuse1;

layout(std140) uniform Camera
{
    vec3 viewPos;
    mat4 view;
    mat4 invView;
    mat4 projection;
    mat4 invProjection;
    mat4 viewProjection;
    mat4 invViewProjection;
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
    vec4 finalColor = texColor * (vec4(lighting, 1.0) + ambientColor);

    FragColor = finalColor;
    //FragColor = vec4(normal, 1.0);
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