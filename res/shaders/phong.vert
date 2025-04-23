#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal; 
layout (location = 2) in vec2 aTexCoords;

out vec3 fragPos;
out vec3 normal;
out vec3 viewPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;
    viewPos = vec3(inverse(view)[3]);

    gl_Position = projection * view * vec4(fragPos, 1.0);
}