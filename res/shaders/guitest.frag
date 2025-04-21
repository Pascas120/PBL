#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;

struct Inner
{
    float flot;
};

struct PointLight {
    vec3 position;
    float constant;
    int array[3];
    Inner inner;
};

uniform PointLight pointLights[2];
uniform sampler2D texture_diffuse1;
uniform vec4 color[2];
uniform vec3 colorPos[2];
uniform bool addColor[2];
uniform mat3 testMat[2];

void main()
{
    float dist = 1.0;
    for (int i = 0; i < 2; i++)
    {
        if (addColor[i] && testMat[i][0].x > 0.0)
        {
            float d = length(FragPos - colorPos[i]);
            dist = min(dist, clamp(d, 0.0, 1.0));
        }
    }
    vec3 position = pointLights[0].position * pointLights[0].constant * pointLights[1].position * pointLights[1].constant;
    int aa = pointLights[0].array[2] + pointLights[1].array[1];
    float flot = pointLights[1].inner.flot;

    FragColor = texture(texture_diffuse1, TexCoords);
}