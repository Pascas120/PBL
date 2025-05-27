#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 texCoords;
out vec3 normal;
out vec3 fragPos;
out vec3 anormal;

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

uniform mat4 model;



void main()
{
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;
    anormal = aNormal;
    texCoords = aTexCoords;
    gl_Position = viewProjection * model * vec4(aPos, 1.0);
}