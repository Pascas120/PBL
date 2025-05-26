#version 330 core
layout (location = 0) in vec3 aPos;


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
    gl_Position = viewProjection * model * vec4(aPos, 1.0);
}